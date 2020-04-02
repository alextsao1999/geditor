#include <SkGraphics.h>
#include <SkImageDecoder.h>
#include "common.h"
#include "geditor.h"
//#include "shlobj_core.h"

void OutputElement(std::wofstream &os, EventContext &ctx);
void OutputEventContext(std::wofstream &os, EventContext &context) {
    for_context(ctx, context) {
        OutputElement(os, context);
    }
}
void OutputElement(std::wofstream &os, EventContext &ctx) {
    if (ctx.tag().contain(TAG("Sub"))) {
        auto *sub = ctx.cast<SubElement>();
        os << sub->content(1) << _GT(" ") << sub->content(0);
        os << _GT(" (");
        auto table = ctx.enter(0).enter(3);
        while (table.has()) {
            auto *row = table.cast<FastRow>();
            os << row->getColumn(1)->m_data << _GT(" ") << row->getColumn(0)->m_data;
            if (!table.isTail()) {
                os << _GT(", ");
            }
            table.next();
        }
        os << _GT(") {") << std::endl;
        OutputEventContext(os, ctx);
        os << _GT("}");
    } else if (ctx.tag().contain(TAG_LINE)) {
        os << ctx.getLineViewer().content() << std::endl;
    } else if (ctx.tag().contain(TAG("Switch"))) {
        OutputEventContext(os, ctx);
    } else if (ctx.tag().contain(TAG("CodeBlock"))) {
        OutputEventContext(os, ctx);
    } else {
        OutputEventContext(os, ctx);
    }

}
void SaveToFile(GEditor *editor) {
/*
    _TCHAR path[255];
    SHGetSpecialFolderPath(nullptr, path, CSIDL_DESKTOPDIRECTORY, 0);
    std::string str(path);
    str += TEXT("/output.c");
    std::wofstream os;
    //auto *cvt = new std::codecvt_byname<wchar_t, char, std::mbstate_t>("Chinese");
    //std::codecvt_utf8_utf16<wchar_t> loc;
    //os.imbue(std::locale(std::locale("Chinese"), &loc));
    os.open(str);
    OutputEventContext(os, editor->m_data->current().m_root);
    os.flush();
    os.close();
*/
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static HWND hButton1, hButton2, hButton3;
    switch (message) {
        case WM_CREATE:
            hButton1 = CreateWindow(TEXT("BUTTON"), TEXT("Add Func"),
                    WS_CHILD | WS_VISIBLE, 10, 5, 120, 30, hWnd, 0, 0, 0);
            hButton2 = CreateWindow(TEXT("BUTTON"), TEXT("Add Local"),
                    WS_CHILD | WS_VISIBLE, 140, 5, 120, 30, hWnd, 0, 0, 0);
            hButton3 = CreateWindow(TEXT("BUTTON"), TEXT("Output"),
                    WS_CHILD | WS_VISIBLE, 270, 5, 120, 30, hWnd, 0, 0, 0);
            return DefWindowProc(hWnd, message, wParam, lParam);
        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED) {
                HWND hObj = (HWND) lParam;
                auto *editor = (GEditor *) GetWindowLongPtr(hWnd, GWL_USERDATA);
                if (hObj == hButton1) {
                    auto *page = (ClassElement *) editor->m_data->current().get(0);
                    auto *sub = new SubElement();
                    sub->append(new AutoLineElement());
                    editor->m_data->current().m_root.enter().insert(sub);
                    editor->m_data->current().m_root.update();
                }
                if (hObj == hButton2) {
                    auto focus = editor->m_data->current().m_root.getCaretManager()->m_context;
                    if (auto *sub = focus->findOuter(TAG("Sub"))) {
                        auto *ele = sub->cast<SubElement>();
                        ele->addLocal(_GT(""), _GT(""));
                        SetFocus(editor->m_hWnd);
                        sub->enter(1).enter(-1).focus();
                        sub->update();
                    }

                }
                if (hObj == hButton3) {
                    //editor->m_data->m_manager.m_client->Exit();
                    //SaveToFile(editor);
                }
            }
            return DefWindowProc(hWnd, message, wParam, lParam);
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
const _TCHAR *CLASSNAME = TEXT("MyWindowClass");
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
    HWND hWnd = CreateWindow(CLASSNAME, TEXT("Code Editor"), WS_OVERLAPPEDWINDOW,
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
/*
    auto *cvt = new std::codecvt_byname<wchar_t, char, std::mbstate_t>("Chinese");
    std::wcout.imbue(std::locale(std::locale("Chinese"), cvt));
*/
    CreateJPEGImageDecoder();
    CreateBMPImageDecoder();
    ASSERT(MyRegisterClass(nullptr), "Register Class Name Error!");
    HWND hwnd = CreateMyWindow();
    auto *g = GEditorBuilder::build(hwnd, 10, 40);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) g);
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
