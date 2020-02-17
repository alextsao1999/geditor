#include <SkGraphics.h>
#include <SkImageDecoder.h>
#include "common.h"
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

void OpenEFile(Document *document) {
    FileBuffer buffer(R"(C:\Users\Administrator\Desktop\edit\f.e)");
    ECodeParser parser(buffer);
    parser.Parse();
    for (auto &mod : parser.code.modules) {
        auto *module = new ModuleElement();
        module->name.append(mod.name.toUnicode());
        document->append(module);
        for (auto &key : mod.include) {
            if (key.type == KeyType::KeyType_Sub) {
                auto *sub = parser.code.find<ESub>(key);
                if (sub) {
                    SubVisitor open(&parser.code, document, module, sub);
                    sub->ast->accept(&open);
                }
            }
        }
    }
    buffer.free();
    document->layout();
    document->m_context.m_renderManager->invalidate();
}
int main() {
    auto *cvt = new std::codecvt_byname<wchar_t, char, std::mbstate_t>("Chinese");
    std::wcout.imbue(std::locale(std::locale("Chinese"), cvt));
    CreateJPEGImageDecoder();
    CreateBMPImageDecoder();
    ASSERT(MyRegisterClass(nullptr), "Register Class Name Error!");
    HWND hwnd = CreateMyWindow();
    auto *g = GEditorBuilder::build(hwnd, 10, 40);
    //OpenEFile(&g->m_data->m_document);
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, nullptr, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    delete g;
    return msg.wParam;
}
