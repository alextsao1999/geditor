//
// Created by Alex on 2019/6/25.
//

#include "document.h"

Context *Document::getContext() { return &m_context; }

void Document::append(RelativeElement *element) {
    element->m_parent = this;
    RelativeElement *ele = m_header;
    while (ele != nullptr && ele->m_next != nullptr)
        ele = ele->m_next;
    if (ele != nullptr) {
        element->m_prev = ele;
        ele->m_next = element;
    } else {
        m_header = element;
    }
}

void Document::flow() { /*m_header->reflow(getContext());*/ }

void Document::FreeAll(RelativeElement *ele) {
    if (ele == nullptr)
        return;
    FreeAll(ele->m_next);
    delete ele;
}

ElementIterator Document::children() {
    return m_elements.begin();
}

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
