//
// Created by Alex on 2020/1/29.
//

#ifndef GEDITOR_AUTO_COMPLETE_H
#define GEDITOR_AUTO_COMPLETE_H

#include "caret_manager.h"
#include <vector>
#include "paint_manager.h"
class AutoComplete {
private:
    HWND m_hWnd;
    WNDPROC OldProc;
    RenderManager m_render;
public:
    struct Item {};
    CaretManager *m_caretManager;
    std::vector<Item> m_items;
    explicit AutoComplete(CaretManager *caret) : m_caretManager(caret) {
        int width = 250, height = 300;
        RegisterACClass();
        auto m_owner = CreateWindowEx(WS_EX_TOPMOST | WS_EX_NOACTIVATE, _GT("AutoComplete"), _GT(""),
                                      WS_POPUP, 0, 0, width, height, 0, nullptr, nullptr, nullptr);
        m_hWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_NOACTIVATE, _GT("ListBox"), _GT("AutoComplete"),
                                LBS_STANDARD | LBS_OWNERDRAWVARIABLE | WS_VSCROLL,
                                0, 0, width, height, m_owner, nullptr, nullptr, nullptr);
        SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR) this);
        //OldProc = WNDPROC(SetWindowLongPtr(m_owner, GWLP_WNDPROC, (LONG_PTR) onWndProc));
        m_render = RenderManager(m_hWnd);
        SendMessage(m_hWnd, LB_SETITEMHEIGHT, 0, 35);
        SendMessage(m_hWnd, LB_ADDSTRING, 0, (LPARAM) _GT("test"));
        SendMessage(m_hWnd, LB_ADDSTRING, 0, (LPARAM) _GT("my test"));
        SendMessage(m_hWnd, LB_ADDSTRING, 0, (LPARAM) _GT("ok"));
    };
    ~AutoComplete() {
        SendMessage(m_hWnd, WM_CLOSE, 0, 0);
    }
    void addItem(const Item &item) {
        m_items.emplace_back(item);
    }
    void clear() {
        m_items.clear();
    }
    void show();
    void hide() {
        ShowWindow(m_hWnd, SW_HIDE);
    }
    static ATOM RegisterACClass() {
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = onWndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = nullptr;
        wcex.hIcon = nullptr;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = _GT("AutoComplete");
        wcex.hIconSm = nullptr;
        return RegisterClassEx(&wcex);
    }

    static LRESULT CALLBACK onWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        auto *ac = (AutoComplete *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (message == WM_DRAWITEM) {
            auto *dis = (DRAWITEMSTRUCT *) lParam;
            printf("%d\n", dis->itemID);
            if (dis->CtlType == ODT_LISTBOX) {
            }
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
        //return CallWindowProc(ac->OldProc, hWnd, message, wParam, lParam);
    }

};


#endif //GEDITOR_AUTO_COMPLETE_H
