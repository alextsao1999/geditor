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
class GEditor;
struct GEditorData {
    HWND m_hwnd{};
    Document m_document;
    PaintManager m_paintManager;
    GEditorData() : m_document(Document(&m_paintManager)) {}
};
class LineElement : public RelativeElement {
    using  RelativeElement::RelativeElement;
public:
    explicit LineElement(int value) : value(value) {}
    int value;
    int height = 25;
    int getLogicHeight(EventContext &context) override {
        return height;
    }
    Display getDisplay() override {
        return Display::Line;
    }
    void redraw(EventContext &context) override {
        Painter painter = context.getPainter();
        LineViewer line = context.getLineViewer();
        painter.setTextColor(RGB(0, 0, 0));
        painter.drawText(4, 4, line.getContent().c_str(), line.getContent().size());
        painter.drawRect(2, 2, getWidth(context) - 40, getHeight(context));
    }

    void onLeftButtonDown(EventContext &context, int x, int y) override {
        context.focus();
        Offset textOffset = getRelOffset(x, y) - Offset(4, 4);
        auto line = context.getLineViewer();
        auto meter = context.getPaintManager()->getTextMeter();
        auto caret = context.getCaretManager();
        caret->data()->index = meter.getTextIndex(line.getContent().c_str(), line.getContent().size(), textOffset.x);
        caret->set(textOffset.x + 4, 4);
        caret->show();
        value++;
        //height += 10;
        context.reflow();
        context.getPaintManager()->refresh();
    }

    void onKeyDown(EventContext &context, int code, int status) override {
        auto meter = context.getPaintManager()->getTextMeter();
        auto caret = context.getCaretManager();
        auto line = context.getLineViewer();
        int x = 0;
        if (code == VK_RIGHT) {
            caret->data()->index++;
            if (caret->data()->index > line.getContent().size())
                caret->data()->index = line.getContent().size();
            x = meter.meterWidth(line.getContent().c_str(), caret->data()->index);
        }
        if (code == VK_LEFT) {
            caret->data()->index--;
            if (caret->data()->index < 0)
                caret->data()->index = 0;
            x = meter.meterWidth(line.getContent().c_str(), caret->data()->index);
        }
        if (code == VK_UP) {

        }
        if (code == VK_DOWN) {
            caret->getEventContext()->next();

        }
        caret->set(x + 4, 4);
        caret->show();

    };

class GEditor {
public:
    GEditorData *m_data;
public:
    GEditor() : m_data(nullptr) {}
    explicit GEditor(HWND parent, int x, int y, int nWidth, int nHeight) {
        m_data = new GEditorData();
        m_data->m_hwnd = CreateWindowEx(0, GEDITOR_CLASSNAME, _GT("GEditor"),
                                        WS_VISIBLE | WS_CHILD | WS_HSCROLL | WS_VSCROLL,
                                        x, y, nWidth, nHeight, parent,
                                        nullptr, nullptr, nullptr);
        if (!m_data->m_hwnd) {
            return;
        }
        SetWindowLong(m_data->m_hwnd, GWL_USERDATA, (LONG) m_data);
        m_data->m_paintManager = PaintManager::FromWindow(m_data->m_hwnd);
        for (int i = 0; i < 1000; ++i) {
            GChar str[255];
            auto line = m_data->m_document.appendLine(new LineElement(i));
            wsprintf(str, _GT("this is test string %d\0"), line.number);
            line.getContent().append(str);
        }
        m_data->m_document.flow();
    }
    ~GEditor() {
        delete m_data;
    }

};

class GEditorBuilder {
public:
    static ATOM init() {
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        wcex.lpfnWndProc = proc;
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
    static GEditor build(HWND parent, int x = 0, int y = 0, int nWidth = 400,int nHeight = 400) {
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
        pos += data->m_paintManager.getViewportOffset(); \
        context.set(&data->m_document, 0); \
        while (context.has()) { \
            if (context.current()->contain(context, pos.x, pos.y)) { \
                context.current()->name(context, pos.x, pos.y); \
                break; \
            } \
            context.next(); \
        }  \
    }
#define MsgCallFocus(name, ...) { \
    auto focus = data->m_document.getContext()->m_caretManager.getFocus(); \
    if (focus) { \
        focus->name(*data->m_document.getContext()->m_caretManager.getEventContext(), ##__VA_ARGS__); \
    } \
}
    /////////////////////////////////////////////////////
    static LRESULT CALLBACK proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        auto data = (GEditorData *) GetWindowLong(hWnd, GWL_USERDATA);
        if (!data) { return DefWindowProc(hWnd, message, wParam, lParam); }
        EventContext context = EventContextBuilder::build(&data->m_document);
        // int idx = (pos.y / data->m_document.getContext()->m_layoutManager.getMinHeight()); \
        // std::cout << "predict : " << idx << std::endl;
        switch (message) {
            case WM_MOUSEWHEEL:
            case WM_VSCROLL:
                handleScroll(SB_VERT, data, hWnd, wParam);
                break;
            case WM_HSCROLL:
                handleScroll(SB_HORZ, data, hWnd, wParam);
                break;
            case WM_SIZE:
                data->m_paintManager.resize();
                break;
            case WM_LBUTTONUP: // 鼠标左键放开
                ReleaseCapture();
                MsgCallEvent(onLeftButtonUp);
                break;
            case WM_LBUTTONDOWN: // 鼠标左键按下
                SetCapture(hWnd);
                SetFocus(hWnd);
                MsgCallEvent(onLeftButtonDown);
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
            case WM_CHAR:
                break;
            case WM_IME_CHAR:

                break;
            case WM_KEYDOWN:
                MsgCallFocus(onKeyDown, wParam, lParam);
                break;
            case WM_KEYUP:
                MsgCallFocus(onKeyUp, wParam, lParam);
                break;
            case WM_PAINT:
                {
                    PAINTSTRUCT ps;
                    HDC hdc = BeginPaint(hWnd, &ps);
                    data->m_paintManager.update();
                    context.set(&data->m_document, 0);
                    while (context.has()) {
                        if (context.current()->getDisplay() != Display::None) {
                            context.current()->redraw(context);
                            context.next();
                        }
                    }
                    RECT rect;
                    GetWindowRect(hWnd, &rect);
                    data->m_paintManager.copy(hdc, rect.right - rect.left, rect.bottom - rect.top);
                    EndPaint(hWnd, &ps);
                }
                break;
            case WM_DESTROY:
                PostQuitMessage(0);
                break;
            case WM_SETFOCUS:
                data->m_document.getContext()->m_caretManager.create();
                break;
            case WM_KILLFOCUS:
                // DestroyCaret();
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }
    static void handleScroll(int nBar, GEditorData *data, HWND hWnd, WPARAM wParam) {
        int prev = GetScrollPos(hWnd, nBar);
        int movement = ((int16_t) HIWORD(wParam)) / -60;
        prev += movement;
        int status = LOWORD(wParam);
        switch (status) {
            case SB_LINEUP:
                prev -= 1;
                break;
            case SB_LINEDOWN:
                prev += 1;
                break;
            case SB_THUMBTRACK:
            {
                SCROLLINFO si;
                si.cbSize = sizeof(SCROLLINFO);
                si.fMask = SIF_ALL;
                GetScrollInfo(hWnd, nBar, &si);
                prev = si.nTrackPos;
            }
                break;
            case SB_PAGEUP:
                prev -= 1;
                break;
            case SB_PAGEDOWN:
                prev += 1;
                break;
            case SB_ENDSCROLL:
                break;
            default:
                break;
        }
        SetScrollPos(hWnd, nBar, prev, true);
        data->m_paintManager.updateViewport(&data->m_document.getContext()->m_layoutManager);
        data->m_document.getContext()->m_caretManager.update();

    }
    static void handleChildren() {

    }

};


#endif //GEDITOR_GEDITOR_H
