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
#define GString std::wstring
struct TextLine {
    TextLine() = default;
    ~TextLine() = default;
    GString content;
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
    inline int getLineNumber() { return m_line; }
    inline bool empty() { return content().empty(); }
    inline GString &content() { return (*m_buffer)[m_line].content; }
    inline const GChar *c_str() { return (const GChar *) content().c_str(); }
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

    template <typename ...Args>
    void format(GChar *text, Args...arg) {
        GChar buf[255];
        gsprintf(buf, text, std::forward<Args>(arg)...);
        append(buf);
    }
};

class TextBuffer {
public:
    LineBuffer m_buffer;
    int m_count = 0;
public:
    TextBuffer() = default;
    inline int getLineCount() { return m_buffer.size(); }
    LineViewer insertLine(int line) {
        if (line >= m_buffer.size()) {
            return getLine(line);
        }
        m_buffer.insert(m_buffer.begin() + line, TextLine());
        return {line, &m_buffer};
    }
    LineViewer appendLine() {
        m_buffer.emplace_back();
        return {(int) m_buffer.size() - 1, &m_buffer};
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
    LineViewer insertLine(LineCounter counter, int offset) {
        if (counter.line >= m_buffer.size()) {
            return getLine(counter.line);
        }
        m_buffer.insert(m_buffer.begin() + counter.line + offset, TextLine());
        return {counter.line + 1, &m_buffer};
    }
    void deleteLine(LineCounter counter, int offset) {
        int line = offset + counter.line;
        if (m_buffer.size() == 1) { return; }
        m_buffer.erase(m_buffer.begin() + line);
    }
    LineViewer getLine(LineCounter counter, int offset, EventContext *context) {
        int line = counter.line + offset;
        if (line >= m_buffer.size()) {
            int rm = line - (int) m_buffer.size() + 1;
            while (rm--) {
                m_buffer.emplace_back();
            }
        }
        return {line, &m_buffer, context};
    }
};

#endif //GEDITOR_TEXT_BUFFER_H
