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
};
class LineElement : public RelativeElement {
    using  RelativeElement::RelativeElement;
public:
    explicit LineElement(int value) : value(value) {}
    int value;
    int height = 20;
    int getLogicHeight(EventContext &context) override {
        return height;
    }
    void dump() override {
        std::cout << "{ x:" << m_offset.x << " y:" << m_offset.y << " }" << std::endl;
    }
    Display getDisplay() override {
        return Display::Block;
    }
    void redraw(EventContext &context) override {
        Painter painter = context.getPainter();
        GChar str[20];
        wsprintf(str, L"我被点击%d下了\0", value);
        painter.drawText(5, 5, str, lstrlenW(str));
        painter.drawRect(5, 5, getWidth(context) - 40, getHeight(context));
    }
    void leftClick(EventContext &context, int x, int y) override {
        value++;
        height += 10;
        context.doc->reflow(context);
        context.doc->getContext()->m_paintManager->refresh();
        std::cout << "{ x:" << x << " y:" << y << " }" << std::endl;

    }
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
        m_data->m_document.getContext()->m_paintManager = &m_data->m_paintManager;
        for (int i = 0; i < 10; ++i) {
            m_data->m_document.append(new LineElement(i));
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
        auto data = (GEditorData *) GetWindowLong(hWnd, GWL_USERDATA);
        if (!data) { return DefWindowProc(hWnd, message, wParam, lParam); }
        EventContext context = EventContextBuilder::build(&data->m_document);
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
            case WM_LBUTTONUP: {
                context.set(&data->m_document, 0);
                Offset pos(LOWORD(lParam), HIWORD(lParam));

                while (context.has()) {
                    if (context.current()->contain(context, pos.x, pos.y)) {
                        Offset offset = pos - context.current()->getOffset();
                        context.current()->leftClick(context, offset.x, offset.y);
                    }
                    context.next();
                }
            }
                break;
            case WM_PAINT:
                {
                    PAINTSTRUCT ps;
                    HDC hdc = BeginPaint(hWnd, &ps);
                    data->m_paintManager.update();
                    context.set(&data->m_document, 0);
                    while (context.has()) {
                        context.current()->redraw(context);
                        context.next();
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
    }

    static void handleChildren() {

    }

};


#endif //GEDITOR_GEDITOR_H
