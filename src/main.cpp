#include <iostream>
#include <SkGraphics.h>
#include "common.h"
#include "text_buffer.h"
#include "geditor.h"
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
const GChar *CLASSNAME = _GT("MyWindowClass");
ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = nullptr;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = CLASSNAME;
    wcex.hIconSm = nullptr;
    return RegisterClassEx(&wcex);
}
auto CreateMyWindow() {
    HWND hWnd = CreateWindow(CLASSNAME, _GT("Code Editor"), WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, nullptr, nullptr);
    if (!hWnd) {
        return hWnd;
    }
    ShowWindow(hWnd, 1);
    UpdateWindow(hWnd);
    return hWnd;
}
void test() {
/*
    TextBuffer buffer;
    auto line = buffer.getLine(0);
    line.content(0).append(_GT("abc"));
    line.content(0).append(_GT("dde"));
    line.content(0).append(_GT("asdf"));
    std::cout << line.content(0).c_str();
    auto line2 = buffer.getLine(50);
    line2.content().append(_GT("this is test string"));
    std::cout << line2.content().c_str();
    line.content().append(L"   ");
    std::cout << line.content().c_str();

    //std::wcout << line.content() << _GT(" -> ") << line.content().size();

    system("pause");
*/

    exit(0);
}

int main() {
    if (MyRegisterClass(nullptr) == 0) {
        MessageBox(nullptr, _GT("注册窗口类名失败"), _GT("错误"), 0);
        exit(1);
    }

    HWND hwnd = CreateMyWindow();
    auto g = GEditorBuilder::build(hwnd);
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, nullptr, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return msg.wParam;
}
