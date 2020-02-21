//
// Created by Administrator on 2019/6/29.
//

#include "text_buffer.h"
#include "event.h"

void LineViewer::insert(int pos, int ch) {
    if (m_context) {
        m_context->push(CommandType::AddChar, CommandData(pos, ch));
    }
    auto &&str = content();
    str.insert(str.begin() + pos, (GChar) ch);
}

void LineViewer::insert(int pos, const GChar *string) {
    if (m_context) {
        int length = gstrlen(string);
        m_context->push(CommandType::AddString, CommandData(pos, length));
    }
    auto &&str = content();
    str.insert(pos, string);
}

void LineViewer::remove(int pos, int length) {
    if (m_context) {
        auto data = CommandData(pos, new GString(c_str() + pos, length));
        m_context->push(CommandType::DeleteString, data);
    }
    auto &&str = content();
    str.erase(str.begin() + pos, str.begin() + pos + length);
}

void LineViewer::remove(int pos) {
    if (m_context) {
        m_context->push(CommandType::DeleteChar, CommandData(pos, charAt(pos)));
    }
    auto &&str = content();
    str.erase(str.begin() + pos);
}

void LineViewer::append(const GChar *text, int len) {
    if (len == 0) {
        len = gstrlen(text);
    }
    if (m_context) {
        m_context->push(CommandType::AddString, CommandData(length(), len));
    }
    content().append(text, len);
}

void LineViewer::clear() {
    if (m_context) {
        auto data = CommandData(0, new GString(content()));
        m_context->push(CommandType::DeleteString, data);
    }
    content().clear();
}
