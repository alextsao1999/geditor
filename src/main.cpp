#include <SkGraphics.h>
#include "common.h"
#include "geditor.h"
#include <libloaderapi.h>
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

int main() {
    ASSERT(MyRegisterClass(nullptr), "Register Class Name Error!");
    HWND hwnd = CreateMyWindow();
    auto g = GEditorBuilder::build(hwnd, 10, 40);
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, nullptr, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return msg.wParam;
}
