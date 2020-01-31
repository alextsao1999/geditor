//
// Created by Alex on 2020/1/29.
//

#include "auto_complete.h"
#include "event.h"
void AutoComplete::show() {
    return;
    RECT rect;
    GetWindowRect(m_caretManager->m_paintManager->m_hWnd, &rect);
    Offset offset = m_caretManager->current() - m_caretManager->m_paintManager->getViewportOffset();
    offset.y += m_caretManager->m_context->height() - 5;
    MoveWindow(m_hWnd, rect.left + offset.x, rect.top + offset.y, 250, 300, true);
    ShowWindow(m_hWnd, SW_SHOW);
    SetFocus(m_caretManager->m_paintManager->m_hWnd);
}
