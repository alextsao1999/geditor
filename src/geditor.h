//
// Created by Alex on 2019/6/28.
//

#ifndef GEDITOR_GEDITOR_H
#define GEDITOR_GEDITOR_H
#include "common.h"
#include "utils.h"
#include "paint_manager.h"
#include "table.h"
#include "document.h"
#include <thread>
static const GChar *GEDITOR_CLASSNAME = _GT("GEditor");
static bool isInit = false;
class GEditor;
struct GEditorData {
    HWND m_hwnd;
    Document m_document;
    RenderManager m_renderManager;
    explicit GEditorData(HWND hwnd) :
    m_hwnd(hwnd),
    m_document(&m_renderManager),
    m_renderManager(hwnd, this){}
};
class GEditor {
public:
    GEditorData *m_data;
public:
    GEditor() :
    m_data(nullptr) {}
    explicit GEditor(HWND parent, int x, int y, int nWidth, int nHeight) {
        HWND hwnd = CreateWindowEx(0, GEDITOR_CLASSNAME, _GT("GEditor"),
                                   WS_VISIBLE | WS_CHILD | WS_HSCROLL | WS_VSCROLL,
                                   x, y, nWidth, nHeight, parent,
                                   nullptr, nullptr, nullptr);
        ASSERT(hwnd, "Create Window Error!");
        m_data = new GEditorData(hwnd);
        SetWindowLongPtr(m_data->m_hwnd, GWLP_USERDATA, (LONG_PTR) m_data);
//        m_data->m_document.append(new MultiLine());
//        m_data->m_document.append(new SingleBlockElement());
        m_data->m_document.append(new SyntaxLineElement());

        m_data->m_document.append(new SyntaxLineElement());
/*
        auto *container = new ScrolledContainer<>(300, 300);
        container->append(new SubElement());
        m_data->m_document.append(container);
*/

//        m_data->m_document.appendElement(new SyntaxLineElement());
//        m_data->m_document.appendElement(new SyntaxLineElement());
        //m_data->m_document.appendElement(new SwitchElement());
        //m_data->m_document.appendElement(new SyntaxLineElement());

//        auto *table = new TableElement(2, 2);
//        auto *table_inner = new TableElement(2, 2);
//        table->replace(0, 0, table_inner);
        //m_data->m_document.appendElement(new ExLineElement());
//        m_data->m_document.append(new MoveElement());
//        m_data->m_document.append(new ButtonElement());
        m_data->m_document.append(new SubElement());
        m_data->m_document.append(new SubElement());
        //m_data->m_document.append(new ScrollElement());

/*
        m_data->m_document.appendElement(table);
        m_data->m_document.appendLine(new LineElement()).append(L"var a = 100;");
        m_data->m_document.appendLine(new SyntaxLineElement()).append(L"class YourClass");
        //m_data->m_document.append(new ButtonElement());
        m_data->m_document.appendElement(new SubElement());
*/
        m_data->m_document.flow();
    }
    ~GEditor() { delete m_data; }
};

class GEditorBuilder {
public:
    static ATOM init() {
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        wcex.lpfnWndProc = onWndProc;
        wcex.cbClsExtra = 0 ;
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
    static GEditor build(HWND parent, int x = 0, int y = 0, int nWidth = 850,int nHeight = 500) {
        if (!isInit && init()) {
            isInit = true;
        }
        if (isInit) {
            return GEditor(parent, x, y, nWidth, nHeight);
        }
        return {};
    }
    ////////////////////////////////////////////////////
#define MsgCallEvent(name) { \
        Offset pos(LOWORD(lParam), HIWORD(lParam)); \
        pos += data->m_renderManager.getViewportOffset(); \
        while (context.has()) { \
            if (context.current()->contain(context, pos.x, pos.y)) { \
                context.current()->name(context, pos.x, pos.y); \
                break; \
            } \
            context.next(); \
        }  \
    }
#define MsgCallFocus(name, ...) { \
        Element *focus = data->m_document.getContext()->m_caretManager.getFocus(); \
        if (focus) \
            focus->name(*data->m_document.getContext()->m_caretManager.getEventContext(), ##__VA_ARGS__); \
    }
    /////////////////////////////////////////////////////
    static LRESULT CALLBACK onWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        auto *data = (GEditorData *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (!data) { return DefWindowProc(hWnd, message, wParam, lParam); }
        EventContext context = data->m_document.m_begin;
        switch (message) {
            case WM_MOUSEMOVE:
                MsgCallEvent(onPreMouseMove);
                if (data->m_document.getContext()->m_selecting) {
                    data->m_document.getContext()->selecting();
                    data->m_renderManager.refresh();
                }
                break;
            case WM_MOUSEWHEEL:
            case WM_VSCROLL:
                onHandleScroll(SB_VERT, data, wParam);
                break;
            case WM_HSCROLL:
                onHandleScroll(SB_HORZ, data, wParam);
                break;
            case WM_SIZE:
                data->m_renderManager.resize();
                break;
            case WM_LBUTTONUP: // 鼠标左键放开
                MsgCallEvent(onLeftButtonUp);
                data->m_document.getContext()->endSelect();
                ReleaseCapture();
                break;
            case WM_LBUTTONDOWN: // 鼠标左键按下
                SetCapture(hWnd);
                SetFocus(hWnd);
                MsgCallEvent(onLeftButtonDown);
                data->m_document.getContext()->startSelect();
                break;
            case WM_LBUTTONDBLCLK: // 鼠标左键双击
                MsgCallEvent(onLeftDoubleClick);
                break;
            case WM_RBUTTONUP:
                MsgCallEvent(onRightButtonUp);
                break;
            case WM_RBUTTONDOWN:
                SetFocus(hWnd);
                MsgCallEvent(onRightButtonDown);
                break;
            case WM_RBUTTONDBLCLK:
                MsgCallEvent(onRightDoubleClick);
                break;
            case WM_IME_CHAR:
            case WM_CHAR:
                if (wParam == 26 && lParam == 2883585) {
                    data->m_document.undo();
                    return 0;
                }
                MsgCallFocus(onInputChar, wParam);
                break;
            case WM_KEYDOWN:
                MsgCallFocus(onKeyDown, wParam, lParam);
                break;
            case WM_KEYUP:
                MsgCallFocus(onKeyUp, wParam, lParam);
                break;
            case WM_PAINT:
                onPaint(hWnd, data);
                break;
            case WM_DESTROY:
                PostQuitMessage(0);
                break;
            case WM_SETFOCUS:
                data->m_document.getContext()->m_caretManager.create();
                break;
            case WM_KILLFOCUS:
                data->m_document.getContext()->m_caretManager.destroy();
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }
    static void onHandleScroll(int nBar, GEditorData *data, WPARAM wParam) {
        int step = (nBar == SB_VERT ? 25 : 1);
        int prev = GetScrollPos(data->m_hwnd, nBar);
        int movement = (((int16_t) HIWORD(wParam)) / -60) * step;
        prev += movement;
        switch (LOWORD(wParam)) {
            case SB_LINEUP:
                prev -= step;
                break;
            case SB_LINEDOWN:
                prev += step;
                break;
            case SB_THUMBTRACK:
            {
                SCROLLINFO si;
                si.cbSize = sizeof(SCROLLINFO);
                si.fMask = SIF_ALL;
                GetScrollInfo(data->m_hwnd, nBar, &si);
                prev = si.nTrackPos;
            }
                break;
            case SB_PAGEUP:
                prev -= step;
                break;
            case SB_PAGEDOWN:
                prev += step;
                break;
            case SB_ENDSCROLL:
                break;
            default:
                break;
        }
        SetScrollPos(data->m_hwnd, nBar, prev, true);
        data->m_renderManager.updateViewport(&data->m_document.getContext()->m_layoutManager);
        data->m_document.getContext()->m_caretManager.update();
    }
    static void onPaint(HWND hWnd, GEditorData *data) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        GRect rect = GRect::MakeLTRB(ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);
        data->m_renderManager.redrawRect(&rect);
        data->m_renderManager.copy();
        EndPaint(hWnd, &ps);
    }
};

#endif //GEDITOR_GEDITOR_H
