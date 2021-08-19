#include <stdio.h>
#include <exception>
#include <lalr/Parser.hpp>
#include <lalr/GrammarCompiler.hpp>
#include <lalr/RegexCompiler.hpp>
#include <iostream>
#include <string>
#include <array>
#include <map>
#include <set>
#include <list>
#include <functional>
#include <Windows.h>
#include <fileapi.h>
#include <memoryapi.h>
using namespace lalr;
template <typename Type = int, int BufferSize = 256, typename SizeType = size_t>
class Stack {
public:
    using Iterator = Type *;
    constexpr Stack() = default;
    constexpr Stack(std::initializer_list<Type> values) {
        for (auto &value : values) {
            emplace_back(value);
        }
    }
    ~Stack() {
        for (auto iter = begin(), last = end(); iter != last; iter++) {
            iter->~Type();
        }
    }
    inline bool empty() noexcept { return m_count; }
    inline SizeType capacity() { return BufferSize; }
    inline Iterator begin() noexcept { return (Type *) m_stacks; }
    inline Iterator end() noexcept { return ((Type *) m_stacks) + m_count; }
    inline SizeType length() noexcept { return m_count; }
    inline SizeType size() noexcept { return m_count; }
    inline Type &front() { return (Type &) m_stacks[0]; }
    inline Type &back() { return (Type &) m_stacks[m_count - 1]; }
    template<class ...ValTy>
    inline void emplace_back(ValTy &&...args) {
        new(begin() + m_count++) Type(std::forward<ValTy>(args)...);
    }
    inline void emplace(Iterator where, const Type &value) {
        std::move_backward(where, end(), end() + 1);
        ::new(where) Type(value);
        m_count++;
    }
    inline void insert(SizeType index, const Type &value) {
        emplace(begin() + index, value);
    }
    inline void erase(SizeType index) {
        Iterator where = begin() + index;
        where->~Type();
        std::move(where + 1, begin() + m_count--, where);
    }
    void push_back(const Type &data) {
        begin()[m_count++] = data;
    }
    void push_back(Type &&data) {
        begin()[m_count++] = std::move(data);
    }
    void pop_back() {
        begin()[--m_count].~Type();
    }
    inline Type &operator[](const SizeType &index) {
        return begin()[index];
    }
private:
    uint8_t m_stacks[BufferSize * sizeof(Type)]{};
    SizeType m_count = 0;
};
bool eat(const char *&token, const char *match) {
    auto len = strlen(match);
    if (memcmp(token, match, len) == 0) {
        token += len;
        return true;
    }
    return false;
}
int testLexer() {
    void *a_or_b;
    RegexCompiler regex;
    regex.compile("(abc)+ok[aghk]okok ", &a_or_b);
    Lexer<const char *> lexer(regex.state_machine(), nullptr);
    lexer.set_action_handler("string", [](const char *begin, const char *end, std::string *lexeme, const void **symbol, const char **position, int *lines) {
        while (**position != ' ') {
            (*position)++;
        }
    });
    const char *str = "asb bbb aaa";
    lexer.reset(str, strlen(str) + str);
    lexer.advance();
    do {
        std::cout << lexer.column() - 1 << " : " << lexer.lexeme() << std::endl;

        lexer.advance();
    } while (lexer.symbol() != nullptr);

    return 0;

}
int testParser() {
    class MyEP : public ErrorPolicy {
        void lalr_error(int line, int column, int error, const char *format, va_list args) override {
            vprintf(format, args);
            printf("  line:%d column:%d\n", line, column);
        }

        void lalr_vprintf(const char *format, va_list args) override {
            vprintf(format, args);
            printf("\n");
        }
    } error;
    const char* calculator_grammar =
            "Cpp {\n"
            "   %whitespace \"[ \\r\\n]*\";\n"
            "   items:  items item | item ;\n"
            "   item:  block | identifier [token(0)] | number [token(0)] | action;\n"
            "   block: '{' items '}' [block(3, 1, 3)] | '{' error [error(0)] | '{' '}' [close(0, 1)];"
            "   action: '[' identifier ']' [token(1)] ;"
            "   number: \"[0-9]*\";\n"
            "   identifier: \"[A-Za-z\\x4e00-\\x9fa5_][A-Za-z0-9\\x4e00-\\x9fa5_]*\";\n"
            "}"
    ;
    GrammarCompiler compiler;
    compiler.compile(calculator_grammar, calculator_grammar + strlen(calculator_grammar), &error);
    using NParser = Parser<const wchar_t *, int>;
    NParser parser(compiler.parser_state_machine());
    //parser.set_debug_enabled(true);
    parser.set_lexer_action_handler("ignore",
            [](const wchar_t *begin, const wchar_t *end,
                    std::wstring *lexeme, const void **symbol, const wchar_t **position, int *lines) {
                lexeme->assign((*position)++, 1);
    });
    parser.set_default_action_handler([&](const int *value, const ParserNode<wchar_t> *nodes, size_t length, const char * id, bool &discard) {
        const char *args = id;
        Stack<> stack;
        int type = 0;
        printf("%s  ", id);
        auto &pp = parser;
        if (eat(args, "token")) {
            type = 1;
        }
        if (eat(args, "add")) {
            type = 2;
        }
        if (eat(args, "error")) {
            //discard = true;
            type = 3;
        }
        args = strchr(args, '(');
        do {
            if (*args == '\0') {
                break;
            }
            while (*args < '0' || *args > '9')
                args++;
            stack.push_back(std::stol(args));
        } while((args = strchr(args, ',')));
        for (auto &arg : stack) {
            printf(" token[%ws] ", nodes[arg].lexeme().c_str());
        }
        printf("\n");
        return 1;
    });
    const wchar_t *input = L"123 asdf 122";
    parser.parse(input, wcslen(input) + input);
    printf("<%d %d> ", parser.full(), parser.accepted());
    //printf("%d\n\n", parser.user_data());

    return 0;
}
//template<class char_t = char>
class PieceTable {
public:
    PieceTable() {
        m_buffers.resize(2);
    }
    class Piece;
    using char_t = char;
    using buffer_idx_t = uint16_t;
    using iter_t = typename std::set<Piece>::iterator;
    using string_t = typename std::basic_string<char_t>;
    using iter_func = std::function<void(const char_t *string, size_t length)>;
    using offset_t = uint32_t;
    enum BufferType {
        Append,
        Insert
    };
    struct Buffer {
        const char_t *map_ptr = nullptr;
        // Append-Only Buffer
        std::unique_ptr<string_t> buffer;
        // Line index in buffer
        std::vector<uint32_t> lines;
        Buffer() {
            buffer = std::make_unique<string_t>();
        }
        Buffer(const char_t *ptr, size_t length) {
            set_map(ptr, length);
        }
        inline void set_map(const char_t *ptr, size_t length) {
            map_ptr = ptr;
            for (size_t index = 0; index < length; ++index) {
                if (ptr[index] == '\n') {
                    lines.push_back(index);
                }
            }
        }
        inline size_t size() { return buffer->size(); }
        inline void append(const string_t &string) {
            size_t offset = buffer->size();
            buffer->append(string);
            for (auto ch : string) {
                if (ch == '\n') {
                    lines.push_back(offset);
                }
                offset++;
            }
        }
        inline const char_t &operator[](const size_t &index) {
            if (map_ptr) {
                return *(map_ptr + index);
            }
            return (*buffer)[index];
        }
    };
    struct Piece {
        buffer_idx_t buffer = Append;
        mutable uint32_t buffer_lines = 0;
        mutable uint32_t buffer_line_offset = 0;
        mutable uint32_t start = 0;
        mutable uint32_t length = 0;
        mutable uint32_t left_lines = 0;
        mutable uint32_t left_length = 0;
        Piece() = default;
        Piece(uint32_t left_length, uint32_t left_lines) : left_length(left_length), left_lines(left_lines) {}
        inline bool operator<(const Piece &rhs) const {
            if (rhs.left_length == ~0) {
                return left_lines < rhs.left_lines;
            }
            return left_length < rhs.left_length;
        }
        void dump() const {
            if (buffer == 0) {
                std::cout << "append ";
            }
            if (buffer == 1) {
                std::cout << "insert ";
            }
            if (buffer > 1) {
                std::cout << "origin ";
            }
            std::cout << "  left_length: "
                      << left_length << "  length:" << length
                      << "  left_lines:" << left_lines << "  lines:" << buffer_lines
                      << "  line_offset:" << buffer_line_offset;
        }
        const Piece &operator=(const Piece&rhs) const {
            buffer_lines = rhs.buffer_lines;
            buffer_line_offset = rhs.buffer_line_offset;
            start = rhs.start;
            length = rhs.length;
            left_lines = rhs.left_lines;
            left_length = rhs.left_length;
            return *this;
        }
    };
    class Iterator {
    private:
        PieceTable *m_piece = nullptr;
        offset_t m_start;
        iter_t iter_start;
        iter_t iter_end;
        iter_t iter;
        offset_t m_remainder = 0;
        const char_t *m_string = nullptr;
        size_t m_length = 0;
    public:
        Iterator() = default;
        Iterator(PieceTable *piece, offset_t start, offset_t end) : m_piece(piece) {
            m_start = start;
            m_remainder = end - start;
            iter = iter_start = m_piece->upper_pos(start);
            iter_end = m_piece->upper_pos(end);
        }
        inline bool empty() { return !m_piece; }
        inline const char_t *c_str() { return m_string; }
        inline size_t length() { return m_length; }
        inline string_t string() { return string_t(m_string, m_length); }
        bool next() {
            if (m_remainder == 0) {
                return false;
            }
            if (iter_start == iter_end) {
                offset_t offset = m_start - iter_start->left_length;
                m_string = &m_piece->m_buffers[iter_start->buffer][iter_start->start + offset];
                m_length = m_remainder;
                m_remainder = 0;
                return true;
            }
            if (iter == iter_start) {
                offset_t offset = m_start - iter->left_length;
                m_string = &m_piece->m_buffers[iter->buffer][iter->start + offset];
                m_length = iter->length - offset;
            } else if (iter == iter_end) {
                m_string = &m_piece->m_buffers[iter->buffer][iter->start];
                m_length = m_remainder;
            } else {
                m_string = &m_piece->m_buffers[iter->buffer][iter->start];
                m_length = iter->length;
            }
            iter++;
            if (m_remainder >= m_length) {
                m_remainder -= m_length;
            } else {
                m_remainder = 0;
            }
            return true;
        }
    };
    size_t size() {
        if (m_pieces.empty()) {
            return 0;
        }
        auto iter = --m_pieces.end();
        return iter->left_length + iter->length;
    }
    size_t lines() {
        if (m_pieces.empty()) {
            return 0;
        }
        auto iter = --m_pieces.end();
        return iter->left_lines + iter->buffer_lines;
    }
    size_t get_line(offset_t pos) {
        auto iter = upper_pos(pos);
        offset_t offset = pos - iter->left_length; // length in the piece
        offset_t buffer_offset = iter->start + offset; // offset in the buffer
        auto &buffer = m_buffers[iter->buffer];
        auto iter_start = buffer.lines.begin() + iter->buffer_line_offset;
        auto iter_end = iter_start + iter->buffer_lines;
        auto find = std::lower_bound(iter_start, iter_end, buffer_offset);
        return iter->left_lines + (find - iter_start);
    }
    size_t line_length(size_t line) {
        return line_end(line) - line_start(line);
    }
    offset_t line_start(size_t line) {
        if (line == 0) {
            return 0;
        }
        return line_end(line - 1) + 1;
    }
    offset_t line_end(size_t line) {
        auto iter = lower_line(line);
        size_t offset = line - iter->left_lines;
        auto &buffer = m_buffers[iter->buffer];
        auto start = buffer.lines[iter->buffer_line_offset + offset];
        return iter->left_length + start - iter->start;
    }
    void append(const string_t &string) {
        Piece piece = feed(string, Append);
        piece.left_lines = lines();
        piece.left_length = size();
        m_pieces.emplace(piece);
    }
    iter_t insert(offset_t pos, const string_t &string) {
        auto iter = split(pos);
        Piece pt = feed(string, Insert);
        pt.left_lines = iter->left_lines;
        pt.left_length = iter->left_length;
        fixup(iter, pt.length, pt.buffer_lines);
        return m_pieces.insert(iter, pt);
    }
    bool erase(offset_t start, offset_t end) {
        auto iter_start = split(start);
        auto iter_end = split(end);
        int32_t delta_length = 0;
        int32_t delta_lines = 0;
        for (auto iter = iter_start; iter != iter_end; iter++) {
            delta_length += iter->length;
            delta_lines += iter->buffer_lines;
        }
        m_pieces.erase(iter_start, iter_end);
        fixup(iter_end, -delta_length, -delta_lines);
        return true;
    }
    inline const char_t &char_at(offset_t pos) {
        auto node = upper_pos(pos);
        size_t offset = pos - node->left_length; // length in the piece
        return m_buffers[node->buffer][node->start + offset];
    }
    inline const char_t &operator[](const size_t &index) { return char_at(index); }
    Iterator iter(offset_t start, offset_t end) {
        return Iterator(this, start, end);
    }
    void iter_range(offset_t start, offset_t end, iter_func func) {
        auto iter_start = upper_pos(start);
        auto iter_end = upper_pos(end);
        offset_t offset = start - iter_start->left_length;
        if (iter_start == iter_end) {
            func(&m_buffers[iter_start->buffer][iter_start->start + offset], end - start);
            return;
        }
        offset_t length = end - (iter_start->left_length + iter_start->length);
        func(&m_buffers[iter_start->buffer][iter_start->start + offset],
             iter_start->length - offset);
        while (length > 0) {
            iter_start++;
            if (length > iter_start->length) {
                func(&m_buffers[iter_start->buffer][iter_start->start], iter_start->length);
                length -= iter_start->length;
            } else {
                func(&m_buffers[iter_start->buffer][iter_start->start], length);
                length = 0;
            }
            if (iter_start == iter_end) {
                break;
            }
        }
    }
    string_t line_string(size_t line) {
        string_t string;
        int start = line_start(line);
        int end = line_end(line);
        string.reserve(end - start);
        iter_range(start, end, [&](const char_t *str, size_t length) {
            string.append(str, length);
        });
        return string;
    }
    string_t range_string(offset_t start, offset_t end) {
        string_t string;
        if (end - start == 0) {
            return string;
        }
        string.reserve(end - start);
        iter_range(start, end, [&](const char_t *str, size_t length) {
            string.append(str, length);
        });
        return string;
    }
    void append_origin(const char_t *map, size_t length) {
        if (!map) {
            return;
        }
        Piece piece;
        piece.buffer = m_buffers.size();
        piece.start = 0;
        piece.length = length;
        piece.left_lines = lines();
        piece.left_length = size();

        m_buffers.emplace_back(map, length);
        auto &back = m_buffers.back();

        piece.buffer_lines = back.lines.size();
        m_pieces.emplace(piece);

    }
    iter_t insert_origin(offset_t pos, const char_t *map, size_t length) {
        auto iter = split(pos);
        Piece pt;
        pt.buffer = m_buffers.size();
        pt.start = 0;
        pt.length = length;

        m_buffers.emplace_back(map, length);
        auto &back = m_buffers.back();
        pt.buffer_lines = back.lines.size();

        pt.left_length = iter->left_length;
        pt.left_lines = iter->left_lines;
        fixup(iter, pt.length, pt.buffer_lines);
        return m_pieces.insert(iter, pt);
    }
    void dump(bool print_line = false, bool print_node_string = false) {
        if (print_line) {
            for (int i = 0; i < lines(); ++i) {
                std::cout << "[" << line_start(i) << ", " << line_end(i) << "] "
                          << i << ": " << line_string(i) << std::endl;
            }
        }
        for (auto &ittt : m_pieces) {
            ittt.dump();
            if (print_node_string) {
                std::string string(&m_buffers[ittt.buffer][ittt.start], ittt.length);
                std::cout << " value:" << string;
            }
            std::cout << std::endl;
        }
    }
private:
    iter_t split(offset_t pos) {
        auto iter = upper_pos(pos);
        if (iter->left_length == pos) {
            return iter;
        }
        if (iter->left_length + iter->length == pos) {
            return --iter;
        }
        Piece pt = *iter;
        iter->length = pos - iter->left_length;
        calc_line(*iter);

        pt.start = pt.start + iter->length;
        pt.length = pt.length - iter->length;
        pt.left_lines = iter->left_lines + iter->buffer_lines;
        pt.left_length = iter->left_length + iter->length;
        pt.buffer_lines -= iter->buffer_lines;
        pt.buffer_line_offset += iter->buffer_lines;
        //calc_line(pt);
        return m_pieces.insert(iter, pt);
    }
    inline iter_t upper_pos(offset_t pos) {
        return --m_pieces.upper_bound(Piece(pos, ~0));
    }
    inline iter_t lower_pos(offset_t pos) {
        return --m_pieces.lower_bound(Piece(pos, ~0));
    }
    inline iter_t upper_line(size_t line) {
        auto iter = m_pieces.upper_bound(Piece(~0, line));
        if (iter == m_pieces.end()) {
            iter = m_pieces.begin();
        }
        return --iter;
    }
    inline iter_t lower_line(size_t line) {
        return find_line(line + 1);
    }
    inline iter_t find_line(size_t line) {
        auto iter = m_pieces.lower_bound(Piece(~0, line));
        if (iter == m_pieces.end()) {
            --iter;
        }
        //!(iter->left_lines < line && line <= iter->left_lines + iter->buffer_lines)
        while (iter->left_lines >= line || iter->buffer_lines == 0) {
            --iter;
        }
        return iter;
    }
    inline void calc_line(const Piece &piece) {
        offset_t buffer_offset = piece.start + piece.length;
        auto iter_begin = m_buffers[piece.buffer].lines.begin() + piece.buffer_line_offset;
        auto iter_end = iter_begin + piece.buffer_lines;
        size_t buffer_lines = std::lower_bound(iter_begin, iter_end, buffer_offset) - iter_begin;
        piece.buffer_lines = buffer_lines;
        /*auto iter = &(m_buffers[piece.buffer][piece.start]);
        auto end = &(m_buffers[piece.buffer][piece.start + piece.length]);
        piece.buffer_lines = 0;
        for (; iter != end; iter++) {
            if (*iter == '\n') {
                piece.buffer_lines++;
            }
        }*/
    }
    inline void fixup(iter_t iter, int32_t delta_length = 0, int32_t delta_lines = 0) {
        auto end = m_pieces.end();
        for (; iter != end; iter++) {
            iter->left_length += delta_length;
            iter->left_lines += delta_lines;
        }
    }
    inline Piece feed(const string_t &string, buffer_idx_t index) {
        Piece piece;
        piece.buffer = index;
        piece.start = m_buffers[piece.buffer].size();
        piece.length = string.length();
        piece.buffer_line_offset = m_buffers[piece.buffer].lines.size();
        m_buffers[piece.buffer].append(string);
        piece.buffer_lines = m_buffers[piece.buffer].lines.size() - piece.buffer_line_offset;
        return piece;
    }
    std::vector<Buffer> m_buffers;
    std::set<Piece> m_pieces;
};
class Origin {
    HANDLE m_hFile, m_hMapping;
    const char *m_pContent = nullptr;
    size_t m_nSize;
public:
    Origin(const char *file) {
        m_hFile = CreateFileA(file, GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        m_hMapping = CreateFileMappingA(m_hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
        m_pContent = (const char *) MapViewOfFile(m_hMapping, FILE_MAP_READ, 0, 0, 0);
        m_nSize = GetFileSize(m_hFile, 0);
    }
    ~Origin() {
        UnmapViewOfFile(m_pContent);
        CloseHandle(m_hMapping);
        CloseHandle(m_hFile);
    }
    const char *ptr() { return m_pContent; }
    size_t size() { return m_nSize; }
};
void check_lu() {
    std::vector<int> list = {10, 20, 30, 40, 50, 60};
    auto check = [&](int i) {
        std::cout << " ---- checking " << i << std::endl;
        auto iter_l = std::lower_bound(list.begin(), list.end(), i);
        auto iter_u = std::upper_bound(list.begin(), list.end(), i);
        std::cout << (iter_l - list.begin())  << ", "  << (iter_u - list.begin()) << std::endl;
        std::cout << *iter_l << "  " << *iter_u << std::endl;
    };
    check(10);
    check(5);
    check(46);
    check(40);

}

void pt_test() {
    //testLexer();
    PieceTable pt;
    //Origin origin("C:\\Users\\Administrator\\Desktop\\test.txt");
    //Origin origin("C:\\Users\\Administrator\\Desktop\\checker.ts");
    //pt.append_origin(origin.ptr(), origin.size());
    pt.append("111\naaaaa\nbbbbbb");

    pt.insert(pt.line_end(0), "\n");
    pt.insert(pt.line_end(1), "test");
    std::cout << pt.lines() << std::endl;
    std::cout << pt.range_string(0, pt.size());
    /*auto tick = GetTickCount();
    for (int i = 0; i < 10; ++i) {
        //pt.insert(pt.line_start(i), "[this is a test string]");
        std::cout << pt.line_string(i) << std::endl;
    }
    std::cout << pt.lines() << std::endl;
    std::cout << pt.line_string(2999) << std::endl;
    std::cout << (GetTickCount() - tick) << "ms"  << std::endl;*/

}

class AutoOffset {
public:
    using offset_t = uint32_t;
    using diff_t = int32_t;
    std::vector<offset_t> m_offsets;
    std::vector<int> m_value;
    offset_t m_anchor = 0;
    diff_t m_length = 0;
    int get_value(offset_t pos) {
        return get_index(pos);
    }
    void set_last(offset_t last) {
        while (m_anchor < m_offsets.size() && ((m_offsets[m_anchor++] += m_length) < last));
    }
    void change(offset_t pos, diff_t length) {
        auto index = get_index(pos);
        if (m_anchor < index) {
            // 插值索引在前面 当前已经到插值索引后面 需要更新之间的索引
            fixup(m_anchor, index, m_length);
            m_anchor = index;
            m_length = m_length + length;
        } else {
            // 插值索引在后面 中间既要增加长度 插值索引后面也需要增加
            fixup(index, m_anchor, length);
            m_length += length;
        }
    }
    void add_value(offset_t pos, int value) {
        m_offsets.emplace_back(pos);
        m_value.emplace_back(value);
        std::sort(m_offsets.begin(), m_offsets.end());
    }
    void fixup(offset_t start_index, offset_t end_index, diff_t delta) {
        auto iter_start = m_offsets.begin() + start_index;
        auto iter_end = m_offsets.begin() + end_index;
        for (auto iter = iter_start; iter != iter_end; iter++) {
            *iter += delta;
        }
    }
    void dump() {
        for (int i = 0; i < m_value.size(); ++i) {
            std::cout << "[" << m_offsets[i] << ", " << m_value[i] << "] ";
        }
        std::cout << std::endl;
    }
private:
    uint32_t get_index(offset_t pos) {
        auto iter = std::lower_bound(m_offsets.begin(), m_offsets.end(), pos);
        return iter - m_offsets.begin();
    }
};
int main(int argc, char *const argv[]) {
/*
    AutoOffset marker;
    marker.add_value(1, 1);
    marker.add_value(3, 2);
    marker.add_value(5, 3);
    marker.add_value(7, 4);
    marker.add_value(12, 4);
    marker.add_value(22, 4);
    marker.add_value(55, 4);
    marker.add_value(66, 4);
    marker.set_last(4);
    marker.change(3, -1);
    marker.dump();
*/
    //marker.set_last(11);

    //marker.dump();


    const char* calculator_grammar =
            "Cpp {\n"
            "   %whitespace \"[ \\r\\n]*\";\n"
            "   items:  items item | item ;\n"
            "   item:  if | ifs;\n"
            "   if: \"if\" [if];\n"
            "   ifs: \"(if)+\" [ifs]"
            "}"
    ;
    GrammarCompiler compiler;
    compiler.compile(calculator_grammar, calculator_grammar + strlen(calculator_grammar));
    Parser<const char *, int> parser(compiler.parser_state_machine());

    parser.set_action_handler("if",
                              [](const int *data, const ParserNode<> *nodes, size_t length, const char *identifier,
                                 bool &discard) {
                                  std::cout << "[if] ";
                                  return 0;
                              });
    parser.set_action_handler("ifs",
                              [](const int *data, const ParserNode<> *nodes, size_t length, const char *identifier,
                                 bool &discard) {
                                  std::cout << "[ifs] ";
                                  return 0;
                              });

            /*("if", [](const int *data, const ParserNode<> *nodes, size_t length, const char *identifier, bool &discard) {
                std::cout << "[if] ";
                 return 0;
             }
            )("ifs", [](const int *data, const ParserNode<> *nodes, size_t length, const char *identifier, bool &discard) {
                std::cout << "[ifs] ";
                return 0;
            });*/
    const char *input = "if ifif if ifif";
    parser.parse(input, strlen(input) + input);

    return 0;
}

