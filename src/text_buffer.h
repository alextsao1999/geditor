//
// Created by Alex on 2019/6/29.
//  先用ArrayMethod 看看效果 之后试试加成内存池和PieceTable
//

#ifndef GEDITOR_TEXT_BUFFER_H
#define GEDITOR_TEXT_BUFFER_H

#include "common.h"
#include "line_index.h"
#include "command_queue.h"
#include <vector>
#include <iostream>
#include <codecvt>
#include <locale>

enum {
    LineFlagBP = 1 << 0,
    LineFlagFold = 1 << 1,
    LineFlagExpand = 1 << 2,
    LineFlagHide = 1 << 3,
    LineFlagLineVert = 1 << 4,
    LineFlagLineHorz = 1 << 5,
    LineFlagArrowDownRight = 1 << 6,


};
struct TextLine {
    TextLine() = default;
    ~TextLine() = default;
    GString m_content;
    int m_flags = 0;
    inline GString &content() { return m_content; }
};
using LineBuffer = std::vector<TextLine>;

class LineViewer {
private:
public:
    int m_line = 0;
    EventContext *m_context = nullptr;
    LineBuffer *m_buffer = nullptr;
    LineViewer() = default;
    LineViewer(int line, LineBuffer *buffer, EventContext *ctx = nullptr) :
    m_line(line), m_buffer(buffer), m_context(ctx) {}
    inline bool empty() { return content().empty(); }
    inline const GChar *c_str() { return (const GChar *) content().c_str(); }
    inline GString &content() { return (*m_buffer)[m_line].m_content; }
    inline int &flags() { return (*m_buffer)[m_line].m_flags; }
    void insert(int pos, int ch);
    void insert(int pos, const GChar *string);
    void remove(int pos, int length);
    void remove(int pos);
    inline int length() { return content().length(); }
    inline size_t size() { return length() * sizeof(GChar); }
    void append(const GChar *text, int length = 0);
    GChar charAt(int pos) { return content()[pos]; }
    GChar &front() { return content().front(); }
    GChar &back() { return content().back(); }
    void clear();
    int getSpaceCount() {
        int count = 0;
        for (auto &ch : content()) {
            if (ch == ' ') {
                count++;
            } else {
                break;
            }
        }
        return count;
    }

    template <int Buffer = 255, typename ...Args>
    void format(const GChar *text, Args...arg) {
        GChar buf[Buffer];
        gsprintf(buf, text, std::forward<Args>(arg)...);
        append(buf);
    }
};

class TextBuffer {
public:
    class CharsIter : public std::iterator<std::input_iterator_tag, GChar, long> {
    private:
        LineBuffer *m_buffer = nullptr;
        int m_line = 0;
        int m_pos = 0;
        int m_length = 0;
    public:
        GChar operator*() const {
            if (m_length == 0) {
                return _GT('\0');
            }
            auto &&content = m_buffer->at(m_line).content();
            if (m_pos >= m_length) {
                return _GT('\n');
            }
            return content[m_pos];
        }
        CharsIter &operator++() {
            if (++m_pos > m_length) {
                m_line++;
                m_pos = 0;
                if (m_line >= m_buffer->size()) {
                    m_length = 0;
                } else {
                    m_length = m_buffer->at(m_line).content().length();
                }
            }
            return *this;
        }
        bool operator==(const CharsIter &rhs) const {
            return m_line == rhs.m_line && m_pos == rhs.m_pos;
        }
        bool operator!=(const CharsIter &rhs) const { return !(*this == rhs); }
        constexpr CharsIter() = default;
        CharsIter(LineBuffer *buffer, int line, int pos) : m_buffer(buffer), m_line(line), m_pos(pos) {
            m_length = m_buffer->at(line).content().length();
            if (m_pos < 0) {
                m_pos += m_length + 1;
            }
        }
    };
    LineBuffer m_buffer;
    TextBuffer() = default;
    inline int getLineCount() const { return m_buffer.size(); }
    void insertLines(int line, int count) {
        for (int i = 0; i < count; ++i) {
            insertLine(line);
        }
    }
    void deleteLines(int line, int count) {
        for (int i = 0; i < count; ++i) {
            deleteLine(line);
        }
    }
    LineViewer appendLine() {
        m_buffer.emplace_back();
        return {(int) m_buffer.size() - 1, &m_buffer};
    }
    LineViewer insertLine(int line) {
        if (line >= m_buffer.size()) {
            return getLine(line);
        }
        m_buffer.insert(m_buffer.begin() + line, TextLine());
        return {line, &m_buffer};
    }
    void deleteLine(int line) {
        if (m_buffer.size() == 1 || line == 0) { return; }
        m_buffer.erase(m_buffer.begin() + line);
    }
    LineViewer getLine(int line) {
        if (line >= m_buffer.size()) {
            int rm = line - (int) m_buffer.size() + 1;
            while (rm--) {
                m_buffer.emplace_back();
            }
        }
        return {line, &m_buffer};
    }
    LineViewer getLine(int line, EventContext *context) {
        if (line >= m_buffer.size()) {
            int rm = line - (int) m_buffer.size() + 1;
            while (rm--) {
                m_buffer.emplace_back();
            }
        }
        return {line, &m_buffer, context};
    }
    LineBuffer &iter() { return m_buffer; }
    CharsIter chars(int line, int pos) {
        return {&m_buffer, line, pos};
    }

};

#endif //GEDITOR_TEXT_BUFFER_H
