//
// Created by Alex on 2020/1/29.
//

#ifndef GEDITOR_AUTO_COMPLETE_H
#define GEDITOR_AUTO_COMPLETE_H

#include "caret_manager.h"
#include <vector>
#include "paint_manager.h"
#include "document.h"
class CompleteDocument : public Document {
public:
    CompleteDocument(RenderManager *render, DocumentManager *mgr);

    void onRedraw(EventContext &context) override {

        Root::onRedraw(context);
    }
};

class CompleterRender : public WindowRenderManager {
public:
    CompleteDocument *m_doc = nullptr;
    CompleterRender(HWND hwnd, CompleteDocument *doc) : WindowRenderManager(hwnd), m_doc(doc) {}
    Document &target() override { return *m_doc; }
};
struct AutoCompleteContext {
    CompleteDocument m_completer;
    CompleterRender m_render;
    explicit AutoCompleteContext(HWND hWnd) :
    m_completer(&m_render, nullptr), m_render(hWnd, &m_completer) {}
};
class AutoComplete {
private:
    HWND m_hWnd;
    std::unique_ptr<AutoCompleteContext> m_context;
    std::vector<CompletionItem> m_items;
public:
    explicit AutoComplete() {
        int width = 250, height = 300;
        RegisterACClass();
        m_hWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_NOACTIVATE, _GT("AutoComplete"), _GT(""),
                                WS_POPUP, 0, 0, width, height, 0, nullptr, nullptr, nullptr);
        SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR) this);
        m_context = std::make_unique<AutoCompleteContext>(m_hWnd);
        show(nullptr);
    };
    ~AutoComplete() {
        SendMessage(m_hWnd, WM_CLOSE, 0, 0);
    }
    void set(std::vector<CompletionItem> &items) {
        m_items = std::move(items);
    }
    void add(const CompletionItem &item) {
        m_items.emplace_back(item);
    }
    void clear() {
        m_items.clear();
    }
    void show(Document *document);
    void hide() {
        return;
        ShowWindow(m_hWnd, SW_HIDE);
    }
    static ATOM RegisterACClass() {
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = onWndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = nullptr;
        wcex.hIcon = nullptr;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = _GT("AutoComplete");
        wcex.hIconSm = nullptr;
        return RegisterClassEx(&wcex);
    }
    static LRESULT CALLBACK onWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam) {
        auto *ac = (AutoComplete *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (!ac || !ac->m_context) {
            return DefWindowProc(hWnd, nMsg, wParam, lParam);
        }
        switch (nMsg) {
            case WM_SIZE:
                ac->m_context->m_render.resize();
                break;
            case WM_PAINT:
                onPaint(hWnd, ac);
                break;
            case WM_DESTROY:
                PostQuitMessage(0);
                break;
            default:
                return DefWindowProc(hWnd, nMsg, wParam, lParam);

        }
        return 0;
    }
    static void onPaint(HWND hWnd, AutoComplete *ac) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        GRect rect = GRect::MakeLTRB(ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);
        ac->m_context->m_render.update(&rect);
        ac->m_context->m_render.copy();
        EndPaint(hWnd, &ps);
    }

};


#endif //GEDITOR_AUTO_COMPLETE_H
