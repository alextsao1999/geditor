//
// Created by Alex on 2019/6/28.
//

#ifndef GEDITOR_GEDITOR_H
#define GEDITOR_GEDITOR_H
#include "common.h"
#include "utils.h"
#include "paint_manager.h"
#include "table.h"
#include "doc_manager.h"
#include "open_visitor.h"
#include "shellapi.h"
static const _TCHAR *GEDITOR_CLASSNAME = TEXT("GEditor");
class EditorRender : public WindowRenderManager {
public:
    GEditorData *m_data = nullptr;
    EditorRender(HWND hwnd, GEditorData *data) : WindowRenderManager(hwnd), m_data(data) {}
    Document &target() override;
};
class GEditor;
struct GEditorData {
    EditorRender m_render;
    DocumentManager m_manager;
    bool m_dragging[2] = {false};
    explicit GEditorData(HWND hwnd) : m_manager(&m_render), m_render(hwnd, this){}
    inline Document &current() { return *m_manager.current(); }

};
class GEditor {
public:
    HWND m_hWnd;
    GEditorData *m_data;
public:
    explicit GEditor(HWND parent, int x, int y, int nWidth, int nHeight) {
        m_hWnd = CreateWindowEx(WS_EX_ACCEPTFILES, GEDITOR_CLASSNAME, TEXT("GEditor"),
                                   WS_VISIBLE | WS_CHILD | WS_HSCROLL | WS_VSCROLL | WS_BORDER,
                                   x, y, nWidth, nHeight, parent,
                                   nullptr, nullptr, nullptr);
        GASSERT(m_hWnd, "Create Window Error!");
        m_data = new GEditorData(m_hWnd);
        SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR) m_data);
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
    static GEditor *build(HWND parent, int x = 0, int y = 0, int nWidth = 850, int nHeight = 500) {
        static bool isInit = false;
        if (!isInit && init()) {
            isInit = true;
        }
        if (isInit) {
            return new GEditor(parent, x, y, nWidth, nHeight);
        }
        return {};
    }
    ////////////////////////////////////////////////////
    #define DispatchEvent(EVENT) { \
        Offset pos = Offset{LOWORD(lParam), HIWORD(lParam)} + current.m_viewportOffset; \
        current.EVENT(current.m_root, pos.x, pos.y); }
    #define MsgCallFocus(name, ...) { \
        auto _context = current.m_context.m_caretManager.m_context; \
        context_on_ptr(_context.ptr(), name, ##__VA_ARGS__); }
    /////////////////////////////////////////////////////
    static int GetDragPosition(HWND hWnd, int nBar) {
        SCROLLINFO si;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_ALL;
        GetScrollInfo(hWnd, nBar, &si);
        return si.nTrackPos;
    }
    static LRESULT CALLBACK onWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam) {
        auto *data = (GEditorData *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (!data || data->m_manager.m_documents.empty()) {
            return DefWindowProc(hWnd, nMsg, wParam, lParam);
        }
        Document &current = data->current();
        switch (nMsg) {
            case WM_MOUSEMOVE:
                {
                    Offset offset = Offset{LOWORD(lParam), HIWORD(lParam)} + current.m_viewportOffset;
                    if (offset != current.m_mouse) {
                        onTrackMouse(hWnd, true);
                    }
                    current.m_mouse = offset;
                }
                current.onMouseMove(current.m_root, current.m_mouse.x, current.m_mouse.y);
                if (current.context()->m_selecting) {
                    current.context()->updateSelect();
                    data->m_render.invalidate();
                }
                break;
            case WM_MOUSEWHEEL:
            case WM_VSCROLL:
                onHandleScroll(hWnd, SB_VERT, data, wParam);
                break;
            case WM_HSCROLL:
                onHandleScroll(hWnd, SB_HORZ, data, wParam);
                break;
            case WM_SIZE:
                data->m_render.resize();
                break;
            case WM_LBUTTONUP: // 鼠标左键放开
                DispatchEvent(onLeftButtonUp);
                current.context()->endSelect();
                ReleaseCapture();
                break;
            case WM_LBUTTONDOWN: // 鼠标左键按下
                SetCapture(hWnd);
                SetFocus(hWnd);
                DispatchEvent(onLeftButtonDown);
                current.context()->startSelect();
                break;
            case WM_LBUTTONDBLCLK: // 鼠标左键双击
                DispatchEvent(onLeftDoubleClick);
                break;
            case WM_RBUTTONUP:
                DispatchEvent(onRightButtonUp);
                break;
            case WM_RBUTTONDOWN:
                SetFocus(hWnd);
                DispatchEvent(onRightButtonDown);
                break;
            case WM_RBUTTONDBLCLK:
                DispatchEvent(onRightDoubleClick);
                break;
            case WM_CHAR:
                if (KeyMap::GetState() & KEY_CTRL) {
                    return 0;
                }
                if (auto *context = current.m_context.m_caretManager.getEventContext()) {
                    SelectionState state = context->getSelectionState();
                    if (state != SelectionNone) {
                        current.m_context.pushStart(PushType::PushTypeSelect);
                        current.onSelectionDelete(current.m_root, state);
                    }
                    MsgCallFocus(onInputChar, state, wParam);
                    if (state != SelectionNone) {
                        current.context()->clearSelect();
                        current.context()->pushEnd();
                        current.context()->m_caretManager.update();
                        current.m_root.redraw();
                    }
                }
                break;
            case WM_KEYDOWN:
                if (current.context()->m_keyMap.hasKey(wParam)) {
                    current.onKeyCommand(wParam);
                    return 0;
                }
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
                CreateCaret(hWnd, nullptr, 2, 17);
                current.m_context.m_caretManager.update();
                ShowCaret(hWnd);
                break;
            case WM_KILLFOCUS:
                DestroyCaret();
                break;
            case WM_MOUSEHOVER:
                onTrackMouse(hWnd, false);
                current.onMouseHover(current.m_root, current.m_mouse.x, current.m_mouse.y);
                break;
            case WM_MOUSELEAVE:
                onTrackMouse(hWnd, false);
                current.m_mouse = {-1, -1};
                current.onMouseLeave(current.m_root, current.m_mouse.x, current.m_mouse.y);
                break;
            case WM_DROPFILES:
                onDropFiles(hWnd, (HDROP) wParam, data);
                break;
            default:
                return DefWindowProc(hWnd, nMsg, wParam, lParam);
        }
        return 0;
    }
    static void onHandleScroll(HWND hWnd, int nBar, GEditorData *data, WPARAM wParam) {
        int step = (nBar == SB_VERT ? 35 : 1);
        int prev = GetScrollPos(hWnd, nBar);
        int movement = (((int16_t) HIWORD(wParam)) / -60) * step;
        prev += movement;
        switch (LOWORD(wParam)) {
            case SB_LINEUP:
                prev -= step;
                break;
            case SB_LINEDOWN:
                data->m_dragging[nBar] = false;
                prev += step;
                break;
            case SB_PAGEUP:
                data->m_dragging[nBar] = false;
                prev -= step;
                break;
            case SB_PAGEDOWN:
                prev += step;
                break;
            case SB_THUMBTRACK:
                data->m_dragging[nBar] = true;
                prev = GetDragPosition(hWnd, nBar);
                break;
            case SB_ENDSCROLL:
            case SB_THUMBPOSITION:
                if (data->m_dragging[nBar]) {
                    prev = GetDragPosition(hWnd, nBar);
                }
                break;
            default:
                break;
        }
        SetScrollPos(hWnd, nBar, prev, true);
        data->m_render.updateViewport();
        data->current().context()->m_caretManager.update();
    }
    static void onDropFiles(HWND hwnd, HDROP hDropInfo, GEditorData *data) {
        char szFileName[MAX_PATH] = "";
        DragQueryFileA(hDropInfo, 0, szFileName, sizeof(szFileName));  //打开拖拽的第一个(下标为0)文件
        FileBuffer buffer(szFileName);
        ECodeParser parser(buffer);
        parser.Parse();
        for (auto &mod : parser.code.modules) {
            auto *module = new ModuleElement();
            module->name.append(mod.name.toUnicode());
            data->current().append(module);
            for (auto &key : mod.include) {
                if (key.type == KeyType::KeyType_Sub) {
                    auto *sub = parser.code.find<ESub>(key);
                    if (sub) {
                        SubVisitor open(&parser.code, &data->current(), module, sub);
                        sub->ast->accept(&open);
                    }
                }
            }
        }
        buffer.free();
        data->current().layout();
        data->m_render.invalidate();
        DragFinish(hDropInfo);
    }
    static void onPaint(HWND hWnd, GEditorData *data) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        GRect rect = GRect::MakeLTRB(ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);
        data->m_render.update(&rect);
        data->m_render.copy();
        EndPaint(hWnd, &ps);
    }
    static void onTrackMouse(HWND hWnd, bool on) {
        static bool trackedMouseLeave = false;
        if (on && !trackedMouseLeave) {
            TRACKMOUSEEVENT csTME;
            csTME.cbSize = sizeof(csTME);
            csTME.dwFlags = TME_LEAVE | TME_HOVER;
            csTME.hwndTrack = hWnd;
            csTME.dwHoverTime = 500;  // 鼠标停留超过 10ms 认为状态为 HOVER
            TrackMouseEvent(&csTME);
        }
        trackedMouseLeave = on;
    }

};

#endif //GEDITOR_GEDITOR_H
