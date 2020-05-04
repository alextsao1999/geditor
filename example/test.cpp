#include <stdio.h>
#include <exception>
#include <lalr/Parser.hpp>
#include <lalr/GrammarCompiler.hpp>
#include <lalr/RegexCompiler.hpp>
#include <iostream>
#include <string>
#include <array>
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
    const wchar_t *input = L"cp { kok kjkl";
    parser.parse(input, wcslen(input) + input);
    printf("<%d %d> ", parser.full(), parser.accepted());
    //printf("%d\n\n", parser.user_data());

    return 0;
}

int main(int argc, char *const argv[]) {
    testLexer();

    return 0;
}

