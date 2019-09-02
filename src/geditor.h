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
    LineElement(int value) : value(value) {}
    int value;
    Offset getOffset() override {
        Offset off = Element::getOffset();
        off.x += 20;
        off.y += 10;
        return off;
    }
    int getLogicHeight(EventContext &context) override {
        return 40;
    }
    void dump() override {
        std::cout << "{ x:" << m_offset.x << " y:" << m_offset.y << " }" << std::endl;
    }
    Display getDisplay() override {
        return Display::Block;
    }
    void redraw(EventContext &context) override {
        Painter painter = context.getPainter();
        painter.drawRect(0, 0, 200, getHeight(context));
        painter.drawText(5, 5, _GT("我来测试一下啊"), 7);
    }
    void leftClick(EventContext &context, int x, int y) override {
        std::cout << "I was Clicked!! " << value << " x:" << x << " y:" << y << std::endl;
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
        if (m_data->m_hwnd) {
            SetWindowLong(m_data->m_hwnd, GWL_USERDATA, (LONG) m_data);
        }
        m_data->m_document.getContext()->m_paintManager = &m_data->m_paintManager;
        m_data->m_document.append(new LineElement(111));
        m_data->m_document.append(new LineElement(222));
        m_data->m_document.append(new LineElement(333));
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
            case WM_LBUTTONUP:
                {
                    //std::cout << "x : " << (lParam & 0xffff) << "  y:" <<  ((lParam >> 16) & 0xffff) << std::endl ;
                    context.set(&data->m_document, 0);
                    while (context.has()) {
                        if(context.current()->contain(context, lParam & 0xffff, (lParam >> 16) & 0xffff)) {
                            Offset offset = context.current()->getOffset();
                            context.current()->leftClick(context, (lParam & 0xffff) - offset.x,
                                                         ((lParam >> 16) & 0xffff) - offset.y);
                        }
                        context.next();
                    }
                }
                break;
            case WM_PAINT:
                {
                    PAINTSTRUCT ps;
                    HDC hdc = BeginPaint(hWnd, &ps);
                    data->m_paintManager.m_HDC = hdc;
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

};


#endif //GEDITOR_GEDITOR_H
