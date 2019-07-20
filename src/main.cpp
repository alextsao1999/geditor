#include <iostream>
#include "document.h"
#include "common.h"
#include "text_buffer.h"
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
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

class LineElement : public RelativeElement {
    using  RelativeElement::RelativeElement;
public:
    int getLogicHeight() override {
        return 20;
    }

    void dump() override {
        std::cout << "{ x:" << m_offset.x << " y:" << m_offset.y << " }" << std::endl;
    }

    Display getDisplay() override {
        return Display::Block;
    }
};
int main() {
    Document doc;
    doc.append(new LineElement());
    doc.append(new LineElement());
    doc.append(new LineElement());
    doc.append(new LineElement());
    doc.append(new LineElement());
    doc.flow();
    auto iter = doc.children();
    while (iter->has()) {
        (*iter)->dump();
        iter->next();
    }
    exit(0);

    if (MyRegisterClass(nullptr) == 0) {
        MessageBox(nullptr, _GT("注册窗口类名失败"), _GT("错误"), 0);
        exit(1);
    }
    CreateMyWindow();
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, nullptr, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return msg.wParam;
}

