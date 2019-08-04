//
// Created by Alex on 2019/6/25.
//

#include "document.h"

Document::Document() { m_context.m_layoutManager = &m_layoutManager; }

Offset Element::getOffset() {
    Offset offset = getLogicOffset();
    if (m_parent != nullptr) {
        Offset base = m_parent->getOffset();
        offset.x += base.x;
        offset.y += base.y;
    }
    return offset;
}

LineViewer EventContext::getLineViewer() {
    return doc->getContext()->m_textBuffer.getLine(index);
}

