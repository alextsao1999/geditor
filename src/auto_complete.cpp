//
// Created by Alex on 2020/1/29.
//

#include "auto_complete.h"
#include "event.h"
#include "document.h"
void AutoComplete::show(Document *document) {
    return;
/*
    auto &caret = document->context()->m_caretManager;
    RECT rect;
    GetWindowRect(caret.m_paintManager->m_hWnd, &rect);
    Offset offset = caret.current();
    offset.y += caret.getEventContext()->height() - 5;
    MoveWindow(m_hWnd, rect.left + offset.x, rect.top + offset.y, 250, 300, true);
    ShowWindow(m_hWnd, SW_SHOW);
    SetFocus(caret.m_paintManager->m_hWnd);
*/
}
