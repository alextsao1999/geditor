//
// Created by Alex on 2019/6/28.
//

#ifndef GEDITOR_GEDITOR_H
#define GEDITOR_GEDITOR_H
#include "common.h"
#include "utils.h"
#include "string.h"
#include "paint_manager.h"
#include "document.h"
static const GChar *GEDITOR_CLASSNAME = _GT("GEditor");
static bool isInit = false;

class GEditor {
private:
    HWND m_hwnd;
    Document m_document;
    PaintManager m_paintManager;
public:
    int test = 100;
    GEditor() : m_hwnd(nullptr) {}
    explicit GEditor(HWND parent, int x, int y, int nWidth, int nHeight) {
        m_hwnd = CreateWindowEx(0, GEDITOR_CLASSNAME, _GT("GEditor"), WS_VISIBLE | WS_CHILD | WS_HSCROLL | WS_VSCROLL,
                                x, y, nWidth, nHeight, parent,
                                nullptr, nullptr, nullptr);
        if (m_hwnd) {
            SetWindowLong(m_hwnd, GWL_USERDATA, (LONG) this);
        }
        m_document.getContext()->m_paintManager = &m_paintManager;

    }

    void show() {
        GChar str[100];
        wsprintf(str, L"测试%d", test);
        MessageBoxW(nullptr, str, _GT("title"), 0);

    }
};

class GEditorBuilder {
public:
    static ATOM init() {
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        wcex.lpfnWndProc = proc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = nullptr;
        wcex.hIcon = nullptr;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = GEDITOR_CLASSNAME;
        wcex.hIconSm = nullptr;
        return RegisterClassEx(&wcex);
    }
    static GEditor build(HWND parent, int x = 0, int y = 0, int nWidth = 400,int nHeight = 400) {
        if (!isInit && init()) {
            isInit = true;
        }
        if (isInit) {
            return GEditor(parent, x, y, nWidth, nHeight);
        }
        return {};
    }
    static LRESULT CALLBACK proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
            case WM_LBUTTONUP:
                {
                    auto *g = (GEditor *) GetWindowLong(hWnd, GWL_USERDATA);
                    g->show();


                }
                break;
            case WM_PAINT:
                {
                    PAINTSTRUCT ps;
                    HDC hdc = BeginPaint(hWnd, &ps);
    //             HDC mem = CreateCompatibleDC(nullptr);
                    SetBkMode(hdc, TRANSPARENT);
                    SetTextColor(hdc, RGB(0, 0, 0));
                    TextOut(hdc, 10, 10, _GT("测试绘图文本"), 6);
                    EndPaint(hWnd, &ps);
                }
                break;
            case WM_DESTROY:
                PostQuitMessage(0);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }

};


#endif //GEDITOR_GEDITOR_H
