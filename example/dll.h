//
// Created by Alex on 2020/1/21.
//

#ifndef GEDITOR_DLL_H
#define GEDITOR_DLL_H

#include "common.h"
#include "geditor.h"
#include "json.hpp"
#include <Windows.h>

using json = nlohmann::json;
#define EXPORT_API extern "C" __declspec(dllexport)
typedef int BOOL;
EXPORT_API GEditor *WINAPI CreateGEditor(HWND hWnd, int x, int y, int width, int height) {
    return GEditorBuilder::build(hWnd, x, y, width, height);
}
EXPORT_API void WINAPI DeleteGEditor(GEditor *editor) {
    delete editor;
}
EXPORT_API HWND WINAPI GEditorGetHWnd(GEditor *editor) { return editor->m_hWnd; }
EXPORT_API void WINAPI GEditorSetOnBlur(GEditor *editor, Document::CallBack callback) {
    editor->m_data->current().m_onBlur = callback;
}
EXPORT_API void WINAPI GEditorSetStyle(GEditor *editor, int style_id, GColor color) {
    GStyle style; //= editor->m_data->current().m_context.m_styleManager.get(style_id);
    style.setColor(color);
    style.setTextSize(14);
    style.setFont("ËÎÌו");
    style.setTextEncoding(SkPaint::TextEncoding::kUTF16_TextEncoding);
    style.setAntiAlias(true);
    style.setBlur(SK_ColorLTGRAY, 3.5f, 0, 0);
    editor->m_data->current().style()->set(style_id, style);
}
EXPORT_API void WINAPI GEditorAddKeyword(GEditor *editor, int style, const char *keyword) {
    editor->m_data->m_manager.m_keywords.emplace(AnsiToUnicode(keyword), style);
}
EXPORT_API void WINAPI GEditorSetColor(GEditor *editor, int style, GColor color) {
    editor->m_data->current().m_context.m_styleManager.get(style).setColor(color);
}
EXPORT_API void WINAPI GEditorRelayout(GEditor *editor) {
    editor->m_data->current().root().relayout();
    editor->m_data->current().root().redraw();
}
EXPORT_API int WINAPI GEditorSendMsg(GEditor *editor, int msg, int param, int value) {
    #define MSG_LoadDcr 0
    #define MSG_LoadBkg 1
    #define MSG_GetBitmap 2
    #define MSG_GetDC 3
    #define MSG_Erase 4
    #define MSG_Redraw 5
    #define MSG_Copy 6
    if (msg == MSG_LoadDcr) {
        CreateJPEGImageDecoder();
        CreateBMPImageDecoder();
    }
    if (msg == MSG_LoadBkg) {
        return SkImageDecoder::DecodeMemory((void *) param, value, &editor->m_data->m_render.m_background);
    }
    if (msg == MSG_GetBitmap) {
        return (int) editor->m_data->m_render.m_hBitmap;
    }
    if (msg == MSG_GetDC) {
        return (int) editor->m_data->m_render.m_hMemDC;
    }
    if (msg == MSG_Erase) {
        auto &render = editor->m_data->m_render;
        render.m_canvas->clear(value);
    }
    if (msg == MSG_Redraw) {
        auto &target = editor->m_data->m_render.target();
        target.onRedraw(target.root());
        target.m_context.m_caretManager.onDraw();
    }
    if (msg == MSG_Copy) {
        editor->m_data->m_render.copy();
    }

    return 0;
}
EXPORT_API void WINAPI GEditorLSPCreate(GEditor *editor, const char *app, const char *cmd) {
    editor->m_data->m_manager.CreateLSP(app, cmd);
}
EXPORT_API void WINAPI GEditorLSPSetHandler(GEditor *editor, Handler response, Handler notify, Handler error) {
    editor->m_data->m_manager.m_handler.m_onResponse = response;
    editor->m_data->m_manager.m_handler.m_onNotify = notify;
    editor->m_data->m_manager.m_handler.m_onError = error;
}
EXPORT_API void WINAPI GEditorLSPInit(GEditor *editor, const char *root) {
    string_ref uri = root;
    auto &client = editor->m_data->m_manager.m_client;
    client->Initialize(uri);
}
EXPORT_API void WINAPI GEditorLSPRequest(GEditor *editor, const char *method, json &json) {
    auto uri = editor->m_data->m_manager.current()->getUri();
    editor->m_data->m_manager.m_client->SendRequest(method, json);
}
EXPORT_API void WINAPI GEditorLSPNotify(GEditor *editor, const char *method, json &json) {
    auto uri = editor->m_data->m_manager.current()->getUri();
    editor->m_data->m_manager.m_client->SendNotify(method, json);
}

EXPORT_API int WINAPI GEditorDocumentOpenFile(GEditor *editor, const char *path) {
    return editor->m_data->m_manager.openFile(path);
}
EXPORT_API int WINAPI GEditorDocumentOpenE(GEditor *editor, const char *path) {
    return editor->m_data->m_manager.open(new EDocument(&editor->m_data->m_manager, path));
}
EXPORT_API int WINAPI GEditorDocumentOpenNew(GEditor *editor) {
    return editor->m_data->m_manager.openNew();
}
EXPORT_API void WINAPI GEditorDocumentClose(GEditor *editor, int index) {
    editor->m_data->m_manager.close(index);
}
EXPORT_API void WINAPI GEditorDocumentChange(GEditor *editor, int index) {
    editor->m_data->m_manager.change(index);
}
EXPORT_API void WINAPI GEditorDocumentGoToDefination(GEditor *editor, Position &pos) {
    editor->m_data->m_manager.onGoToDefinition(pos);
}
EXPORT_API void WINAPI GEditorDocumentGoToDeclaration(GEditor *editor, Position &pos) {
    editor->m_data->m_manager.onGoToDeclaration(pos);
}
EXPORT_API void WINAPI GEditorDocumentHover(GEditor *editor, Position &pos) {
    editor->m_data->m_manager.onHover(pos);
}
EXPORT_API void WINAPI GEditorDocumentSignatureHelp(GEditor *editor, Position &pos) {
    editor->m_data->m_manager.onSignatureHelp(pos);
}
EXPORT_API void WINAPI GEditorDocumentCompletion(GEditor *editor, Position &pos) {
    CompletionContext ctx;
    ctx.triggerKind = CompletionTriggerKind::Invoked;
    editor->m_data->m_manager.onComplete(pos, ctx);
}
EXPORT_API void WINAPI GEditorDocumentSymbolInfo(GEditor *editor, Position &pos) {
    auto uri = editor->m_data->m_manager.current()->getUri();
    editor->m_data->m_manager.m_client->SymbolInfo(uri, pos);
}
EXPORT_API void WINAPI GEditorWorkspaceSymbol(GEditor *editor, const char *query) {
    editor->m_data->m_manager.m_client->WorkspaceSymbol(query);
}
EXPORT_API void WINAPI GEditorDocumentSymbol(GEditor *editor) {
    auto uri = editor->m_data->m_manager.current()->getUri();
    editor->m_data->m_manager.m_client->DocumentSymbol(uri);
}
EXPORT_API void WINAPI GEditorDocumentRename(GEditor *editor, Position &pos, const char *name) {
    auto uri = editor->m_data->m_manager.current()->getUri();
    editor->m_data->m_manager.m_client->Rename(uri, pos, name);
}
EXPORT_API void WINAPI GEditorDocumentReferences(GEditor *editor, Position &pos) {
    auto uri = editor->m_data->m_manager.current()->getUri();
    editor->m_data->m_manager.m_client->References(uri, pos);
}
EXPORT_API void WINAPI GEditorDocumentTypeHierarchy(GEditor *editor, Position &pos, int d, int resolve) {
    auto uri = editor->m_data->m_manager.current()->getUri();
    editor->m_data->m_manager.m_client->TypeHierarchy(uri, pos, (TypeHierarchyDirection) d, resolve);
}
EXPORT_API void WINAPI GEditorDocumentHighlight(GEditor *editor, Position &pos) {
    auto uri = editor->m_data->m_manager.current()->getUri();
    editor->m_data->m_manager.m_client->DocumentHighlight(uri, pos);
}
EXPORT_API void WINAPI GEditorDocumentColor(GEditor *editor) {
    auto uri = editor->m_data->m_manager.current()->getUri();
    editor->m_data->m_manager.m_client->DocumentColor(uri);
}
EXPORT_API void WINAPI GEditorDocumentFormatting(GEditor *editor, Position &pos) {
    auto uri = editor->m_data->m_manager.current()->getUri();
    editor->m_data->m_manager.m_client->Formatting(uri);
}
EXPORT_API void WINAPI GEditorDocumentPushStart(GEditor *editor) {
    editor->m_data->current().context()->pushStart(PushType::PushTypeNone);
}
EXPORT_API void WINAPI GEditorDocumentPushEnd(GEditor *editor) {
    editor->m_data->current().context()->pushEnd();
}
EXPORT_API void WINAPI GEditorDocumentApplyChange(GEditor *editor, json &value) {
    Range range = value["range"];
    if (range.start.line == range.end.line) {

    }
    EventContextRef ref(editor->m_data->current().root().findLine(range.start.line));

}

CaretPos PositionToCaretPos(GEditor *editor, Position &positon, bool focus = false) {
    EventContext *ctx = editor->m_data->current().root().findLine(positon.line);
    CaretPos pos = TextCaretService::GetCaretPos(*ctx, {4, 6}, positon.character);
    if (focus) {
        ctx->pos().setIndex(positon.character);
        ctx->focus(false);
    } else {
        ctx->free();
    }
    return pos;
}
EXPORT_API void WINAPI GEditorSelect(GEditor *editor, Position &start, Position &end) {
    //offset = editor->m_data->current().context()->m_caretManager.current();
    auto &caret = editor->m_data->current().context()->m_caretManager;
    CaretPos from = PositionToCaretPos(editor, start);
    CaretPos to = PositionToCaretPos(editor, end, true);
    editor->m_data->current().context()->select(from, to);
}
EXPORT_API void WINAPI GEditorCaretOffset(GEditor *editor, Offset &offset) {
    offset = editor->m_data->current().context()->m_caretManager.current();
}

EXPORT_API EventContext *WINAPI GEditorGetFocus(GEditor *editor) {
    return editor->m_data->current().m_context.m_caretManager.getEventContext();
}
EXPORT_API EventContext *WINAPI GEditorGetRoot(GEditor *editor) {
    return &editor->m_data->current().root();
}
EXPORT_API Document *WINAPI GEditorGetDocument(GEditor *editor) {
    return &editor->m_data->current();
}

EXPORT_API int WINAPI GEditorGetLineCount(GEditor *editor) {
    return editor->m_data->current().buffer()->getLineCount();
}
EXPORT_API void WINAPI GEditorLineSetFlags(GEditor *editor, int line, int flags) {
    LineViewer viewer = editor->m_data->current().buffer()->getLine(line);
    viewer.flags() = flags;
}
EXPORT_API int WINAPI GEditorLineGetFlags(GEditor *editor, int line) {
    LineViewer viewer = editor->m_data->current().buffer()->getLine(line);
    return viewer.flags();
}

EXPORT_API void WINAPI GEditorLineInsert(GEditor *editor, int line, const char *string) {
    auto viewer = editor->m_data->current().buffer()->insertLine(line);
    if (string) {
        viewer.append(A2W(string));
    }
}
EXPORT_API void WINAPI GEditorLineInsertW(GEditor *editor, int line, const wchar_t *string) {
    auto viewer = editor->m_data->current().buffer()->insertLine(line);
    if (string) {
        viewer.append(A2W(string));
    }
}
EXPORT_API int WINAPI GEditorLineAppend(GEditor *editor, const char *string) {
    LineViewer line = editor->m_data->current().buffer()->appendLine();
    if (string != nullptr) {
        line.content().assign(A2W(string));
    }
    return line.m_line;
}
EXPORT_API int WINAPI GEditorLineAppendW(GEditor *editor, const wchar_t *string) {
    LineViewer line = editor->m_data->current().buffer()->appendLine();
    if (string) {
        line.content().assign(string);
    }
    return line.m_line;
}
EXPORT_API void WINAPI GEditorLineDelete(GEditor *editor, int line) {
    editor->m_data->current().buffer()->deleteLine(line);
}
EXPORT_API void WINAPI GEditorLineSetString(GEditor *editor, int line, const char *string) {
    LineViewer viewer = editor->m_data->current().buffer()->getLine(line);
    if (string) {
        viewer.content().assign(A2W(string));
    }
}
EXPORT_API void WINAPI GEditorLineSetStringW(GEditor *editor, int line, const wchar_t *string) {
    LineViewer viewer = editor->m_data->current().buffer()->getLine(line);
    if (string) {
        viewer.content().assign(string);
    }
}
EXPORT_API int WINAPI GEditorLineGetLength(GEditor *editor, int line) {
    auto &string = editor->m_data->current().buffer()->getLine(line).content();
    return WideCharToMultiByte(0, 0, &string.front(), string.length(), 0, 0, 0, 0);
}
EXPORT_API int WINAPI GEditorLineGetLengthW(GEditor *editor, int line) {
    auto &string = editor->m_data->current().buffer()->getLine(line).content();
    return string.length();
}
EXPORT_API void WINAPI GEditorLineGetString(GEditor *editor, int line, char *string, int length) {
    auto &content = editor->m_data->current().buffer()->getLine(line).content();
    if (content.length() == 0) {
        return;
    }
    WideCharToMultiByte(0, 0, &content.front(), content.length(), string, length, 0, 0);
}
EXPORT_API const wchar_t *WINAPI GEditorLineGetStringW(GEditor *editor, int line) {
    auto &content = editor->m_data->current().buffer()->getLine(line).content();
    return content.c_str();
}
EXPORT_API Element *WINAPI GEditorAppendElement(GEditor *editor, Element *element) {
    return editor->m_data->current().append(element);
}
EXPORT_API void WINAPI EventContextDump(EventContext *context) {
    auto str = context->path();
    MessageBoxA(nullptr, str.c_str(), "Dump Result", 0);
}

EXPORT_API void WINAPI EventContextLineStringInsert(EventContext *context, int offset, int pos, const char *str) {
    context->getLineViewer(offset).insert(pos, A2W(str));
}
EXPORT_API void WINAPI EventContextLineStringAppend(EventContext *context, int offset, const char *str) {
    context->getLineViewer(offset).append(A2W(str));
}
EXPORT_API void WINAPI EventContextLineStringRemove(EventContext *context, int offset, int pos, int length) {
    context->getLineViewer(offset).remove(pos, length);
}
EXPORT_API void WINAPI EventContextLineStringClear(EventContext *context, int offset) {
    context->getLineViewer(offset).clear();
}
EXPORT_API void WINAPI EventContextSetLineString(EventContext *context, int offset, const char *str, BOOL pushCommand) {
    auto &string = context->getLineViewer(offset).content();
    if (pushCommand) {
        context->push(CommandType::SetString, CommandData(0, new GString(string)));
    }
    string.assign(A2W(str));
}
EXPORT_API int WINAPI EventContextGetLineLength(EventContext *context, int offset) {
    auto &string = context->getLineViewer(offset).content();
    return WideCharToMultiByte(0, 0, &string.front(), string.length(), 0, 0, 0, 0);
}
EXPORT_API void WINAPI EventContextGetLineString(EventContext *context, int offset, char *string, int length) {
    auto &content = context->getLineViewer(offset).content();
    if (content.length() == 0) {
        return;
    }
    WideCharToMultiByte(0, 0, &content.front(), content.length(), string, length, 0, 0);
}

EXPORT_API void WINAPI EventContextGetPosition(EventContext *context, Position *position) {
    *position = context->position();
}
EXPORT_API EventContext *WINAPI EventContextCopy(EventContext *context) {
    return context->copy();
}
EXPORT_API void WINAPI EventContextInsert(EventContext *context, Element *element, BOOL push) {
    context->insert(element, push);
}
EXPORT_API void WINAPI EventContextRemove(EventContext *context, BOOL push) {
    context->remove(push);
}
EXPORT_API void WINAPI EventContextEnter(EventContext *enter, EventContext *context, int index) {
    *enter = context->enter(index);
}
EXPORT_API void WINAPI EventContextNearby(EventContext *ctx, EventContext *context, int index) {
    *ctx = context->nearby(index);
}
EXPORT_API BOOL WINAPI EventContextNext(EventContext *context) {
    return context->next();
}
EXPORT_API BOOL WINAPI EventContextPrev(EventContext *context) {
    return context->prev();
}
EXPORT_API void WINAPI EventContextTag(EventContext *context, char *string) {
    strcpy(string, context->tag().str);
}
EXPORT_API void WINAPI EventContextRelayout(EventContext *context) {
    context->relayout();
}
EXPORT_API void WINAPI EventContextUpdate(EventContext *context) {
    context->update();
}
EXPORT_API void WINAPI EventContextRedraw(EventContext *context) {
    context->redraw();
}
EXPORT_API void WINAPI EventContextReflow(EventContext *context) {
    context->reflow();
}
EXPORT_API void WINAPI EventContextFocus(EventContext *context, BOOL isCopy, BOOL focus) {
    context->focus(isCopy, focus);
}
EXPORT_API BOOL WINAPI EventContextCanEnter(EventContext *context) {
    return context->canEnter();
}
EXPORT_API BOOL WINAPI EventContextIsHead(EventContext *context) {
    return context->isHead();
}
EXPORT_API BOOL WINAPI EventContextIsTail(EventContext *context) {
    return context->element->getNext() == nullptr;
}
EXPORT_API int WINAPI EventContextGetLine(EventContext *context) {
    return context->getLineViewer().m_line;
}
EXPORT_API int WINAPI EventContextGetLineCount(EventContext *context) {
    return context->element->getLineNumber();
}
EXPORT_API void WINAPI EventContextRepalce(EventContext *context, Element *new_ele) {
    context->replace(new_ele);
}
EXPORT_API void WINAPI EventContextGetPos(EventContext *context, CaretPos *pos) {
    *pos = context->pos();
}
EXPORT_API void WINAPI EventContextSetPos(EventContext *context, CaretPos *pos) {
    context->setPos(*pos);
    context->document()->caret()->update();
}
EXPORT_API EventContext *WINAPI EventContextFindPrev(EventContext *context, char *tag) {
    return context->findPrev(tag);
}
EXPORT_API EventContext *WINAPI EventContextFindNext(EventContext *context, char *tag) {
    return context->findNext(tag);
}
EXPORT_API EventContext *WINAPI EventContextFindInner(EventContext *context, char *tag) {
    return context->findInnerFirst(tag);
}
EXPORT_API EventContext *WINAPI EventContextFindOuter(EventContext *context, char *tag) {
    return context->findOuter(tag);
}
EXPORT_API EventContext *WINAPI EventContextFindLine(EventContext *context, int line) {
    return context->findLine(line);
}
EXPORT_API EventContext *WINAPI EventContextGetOuter(EventContext *context, int count) {
    return context->getOuter(count);
}
EXPORT_API BOOL WINAPI EventContextGetRect(EventContext *context, SkIRect *rect) {
    if (rect) {
        *rect = context->viewportRect().round();
    }
    return context->visible();
}

EXPORT_API void WINAPI EventContextFree(EventContext *context) {
    context->free();
}
EXPORT_API void WINAPI FreeElement(Element *element) {
    element->free();
}

EXPORT_API Element *WINAPI CreateModuleElement(const char *string) {
    auto *ret = new ModuleElement();
    ret->name.assign(A2W(string));
    return ret;
}
EXPORT_API Element *WINAPI CreateClassElement() {
    return new ClassElement();
}

EXPORT_API Element *WINAPI CreateSubElement() {
    return new SubElement();
}
EXPORT_API Element *WINAPI CreateLineElement() {
    return new AutoLineElement();
}
EXPORT_API Element *WINAPI CreateIfElement(int count) {
    return new SingleBlockElement(count);
}
EXPORT_API Element *WINAPI CreateBlockElement(int count) {
    return new CodeBlockElement(count);
}
EXPORT_API Element *WINAPI CreateLoopElement(int count) {
    return new LoopBlockElement(count);
}
EXPORT_API Element *WINAPI CreateSwitchElement(int count) {
    return new SwitchElement(count);
}
class MyTable : public TableElement {
public:
    Tag m_tag;
    using TableElement::TableElement;
    Tag getTag(EventContext &context) override {
        if (!m_tag.empty()) { return m_tag; }
        return FastTable::getTag(context);
    }
};
EXPORT_API Element *WINAPI CreateTableElement(int row, int column) {
    auto *pt = new MyTable(row, column);
    //pt->payload = payload;
    return new MyTable(row, column);
}
EXPORT_API Element *WINAPI CreateRowElement(int column) {
    return new FastRow(column);
}
EXPORT_API void WINAPI TableSetTop(TableElement *table, int top) {
    table->m_top = top;
}
EXPORT_API int WINAPI TableGetTop(TableElement *table) {
    return table->m_top;
}
EXPORT_API void WINAPI TableSetIdentifier(TableElement *table, int top) {
    table->payload = top;
}
EXPORT_API int WINAPI TableGetIdentifier(TableElement *table) {
    return table->payload;
}

EXPORT_API void WINAPI TableSetColorProvider(TableElement *table, TableElement::ColorProvider p) {
    table->m_provider = p;
}

EXPORT_API void WINAPI TableSetTag(MyTable *table, const char *str) {
    table->m_tag = str;
}

EXPORT_API void WINAPI RowSetColumnString(FastRow *row, int column, const char *string) {
    row->getColumn(column)->setContent(A2W(string));
}
EXPORT_API void WINAPI RowSetHeader(FastRow *row, BOOL header) {
    row->setHeader(header);
}
EXPORT_API void WINAPI RowSetUndeleteable(FastRow *row, BOOL de) {
    row->setUndeleteable(de);
}
EXPORT_API int WINAPI RowGetColumnLength(FastRow *row, int column) {
    auto &string = row->getColumn(column)->m_data;
    return WideCharToMultiByte(0, 0, &string.front(), string.length(), 0, 0, 0, 0);
}
EXPORT_API void WINAPI RowGetColumnString(FastRow *row, int column, char *string, int length) {
    auto &content = row->getColumn(column)->m_data;
    if (content.length() == 0) {
        return;
    }
    WideCharToMultiByte(0, 0, &content.front(), content.length(), string, length, 0, 0);
}
EXPORT_API void WINAPI RowSetColumnRadio(FastRow *row, int column, BOOL radio) {
    row->getColumn(column)->m_radio = radio;
}
EXPORT_API BOOL WINAPI RowGetColumnRadio(FastRow *row, int column) {
    return row->getColumn(column)->m_radio;
}
EXPORT_API int WINAPI RowGetColumnCount(FastRow *row) {
    return row->m_items.size();
}

EXPORT_API void WINAPI SubSetContent(SubElement *sub , int column, const char *name) {
    sub->content(column).assign(A2W(name));
}
EXPORT_API FastRow *WINAPI SubAddParam(SubElement *sub, const char *name, const char *type) {
    return sub->addParam(A2W(name), A2W(type));
}
EXPORT_API FastRow *WINAPI SubAddLocal(SubElement *sub, const char *name, const char *type) {
    return sub->addLocal(A2W(name), A2W(type));
}

EXPORT_API Element *WINAPI ElementGetChild(Container<> *element, int index) {
    return element->get(index);
}
EXPORT_API void WINAPI ElementAppendChild(Container<> *element, Element *add) {
    element->append(add);
}
EXPORT_API void WINAPI ElementReplaceChild(Container<> *element, int index, Element *new_ele) {
    element->replace(index, new_ele)->free();
}
EXPORT_API int WINAPI ElementGetChildCount(Container<> *element) {
    return element->getChildCount();
}

#include "GrammerParser.h"

using LLGrammer = GrammerParser<char>;

EXPORT_API LLGrammer *WINAPI CreateLLGrammer(const char *grammer) {
    auto *g = new LLGrammer();
    if (g->compile(grammer)) {
        return g;
    }
    delete g;
    return nullptr;
}
EXPORT_API void WINAPI DeleteLLGrammer(LLGrammer *grammer) {
    delete grammer;
}
EXPORT_API json *WINAPI LLGrammerParse(LLGrammer *grammer, const char *string, const char *rule_name) {
    LLGrammer::GrammerParserPtr rule = nullptr;
    if (string == nullptr || strlen(rule_name) == 0) {
        rule = grammer->get_default();
    } else {
        rule = grammer->m_rules[rule_name];
    }
    auto *value = new json();
    LLGrammer::GrammerBuilder::Lexer lexer;
    lexer.reset(string, strlen(string));
    rule->init(lexer);
    rule->parse(lexer, *value);
    return value;
}

EXPORT_API json *WINAPI JsonCreate() {
    return new json();
}
EXPORT_API json *WINAPI JsonParse(const char *str) {
    auto *value = new json();
    if (str != nullptr) {
        *value = json::parse(str);
    }
    return value;
}
EXPORT_API void WINAPI JsonFree(json *value) {
    delete value;
}
EXPORT_API int WINAPI JsonGetType(json &value) { return (int) value.type(); }
EXPORT_API const char *WINAPI JsonGetTypeName(json &value) { return value.type_name(); }
EXPORT_API json *WINAPI JsonAtPath(json &value, const char *str) {
    return &value[json::json_pointer(str)];
}
EXPORT_API json *WINAPI JsonAtIndex(json &value, int index) {
    return &value[index];
}
EXPORT_API json *WINAPI JsonAtKey(json &value, const char *key) {
    return &value[key];
}
EXPORT_API BOOL WINAPI JsonContainsPath(json &value, const char *str) {
    return value.contains(json::json_pointer(str));
}
EXPORT_API BOOL WINAPI JsonContainsKey(json &value, const char *key) {
    return value.contains(key);
}
EXPORT_API void WINAPI JsonEraseIndex(json &value, int index) {
    value.erase(value.begin() + index);
}
EXPORT_API void WINAPI JsonErasePath(json &value, const char *path) {
    value.erase(json::json_pointer(path));
}
EXPORT_API void WINAPI JsonEraseKey(json &value, const char *key) {
    value.erase(key);
}
EXPORT_API json *WINAPI JsonInsert(json &value, int index) {
    value.insert(value.begin() + index, json());
    return &value[index];
}
EXPORT_API json *WINAPI JsonAppend(json &value) {
    return &value.emplace_back();
}
EXPORT_API BOOL WINAPI JsonIsNull(json &value) {
    return value.is_null();
}
EXPORT_API int WINAPI JsonSize(json &value) { return value.size(); }
EXPORT_API int WINAPI JsonGetInt(json &value) { return value.get<int>(); }
EXPORT_API int64_t WINAPI JsonGetInt64(json &value) { return value.get<int64_t>(); }
EXPORT_API float WINAPI JsonGetFloat(json &value) { return value.get<float>(); }
EXPORT_API BOOL WINAPI JsonGetBool(json &value) { return value.get<bool>(); }
EXPORT_API const char *WINAPI JsonGetString(json &value) { return value.get_ref<std::string &>().c_str(); }
EXPORT_API void WINAPI JsonSet(json &value, json *v) { value = std::move(*v);delete v; }
EXPORT_API void WINAPI JsonMove(json &value, json &v) { value = std::move(v); }
EXPORT_API void WINAPI JsonCopy(json &value, json &v) { value = v; }
EXPORT_API void WINAPI JsonSetInt(json &value, int v) { value = v; }
EXPORT_API void WINAPI JsonSetInt64(json &value, int64_t v) { value = v; }
EXPORT_API void WINAPI JsonSetFloat(json &value, float v) { value = v; }
EXPORT_API void WINAPI JsonSetBool(json &value, BOOL v) { value = v; }
EXPORT_API void WINAPI JsonSetString(json &value, const char *v) { value = v; }
EXPORT_API std::string *WINAPI JsonDump(json &value, int indent) {
    return new std::string(value.dump(indent));
}
EXPORT_API const char *WINAPI JsonDumpString(std::string *value) { return value->c_str(); }
EXPORT_API void WINAPI JsonDumpFree(std::string *value) { delete value; }

#include <lalr/GrammarCompiler.hpp>
#include <lalr/Parser.ipp>
using namespace lalr;
struct DGrammer {
    typedef void (*ReporterHandler)(DGrammer *, const char *, int, int);
    GrammarCompiler grammer;
    class ErrorReporter : public ErrorPolicy {
    public:
        DGrammer *m_this = nullptr;
        ReporterHandler m_handler = nullptr;
        void lalr_error(int line, int column, int error, const char *format, va_list args) override {
            char buffer[2048];
            vsprintf(buffer, format, args);
            if (m_handler) {
                m_handler(m_this, buffer, line, column);
            }
        }
        void lalr_vprintf(const char *format, va_list args) override {
            char buffer[2048];
            vsprintf(buffer, format, args);
            if (m_handler) {
                m_handler(m_this, buffer, 0, 0);
            }
        }
    } reporter;
};
using DataType = void *;
using DNodes = ParserNode<>;
using DParser = Parser<const char *, DataType>;
typedef DataType (WINAPI *ParserHandler)(const DataType *, const DNodes *, int , const char *);
EXPORT_API DGrammer *WINAPI CreateGrammer(const char *grammer, DGrammer::ReporterHandler handler) {
    auto *res = new DGrammer();
    res->reporter.m_this = res;
    res->reporter.m_handler = handler;
    if (res->grammer.compile(grammer, grammer + strlen(grammer), &res->reporter)) {
        delete res;
        return nullptr;
    }
    return res;
}
EXPORT_API void WINAPI DeleteGrammer(DGrammer *grammer) {
    delete grammer;
}
EXPORT_API DParser *WINAPI CreateParser(DGrammer *grammer) {
    return new DParser(grammer->grammer.parser_state_machine());
}
EXPORT_API void WINAPI DeleteParser(DParser *parser) {
    delete parser;
}
EXPORT_API void WINAPI ParserAddHandler(DParser &parser, const char *name, ParserHandler handler) {
    DParser::ParserActionFunction fun = [=](const DataType *data, const DNodes *nodes, size_t length, const char *id, bool &discard) {
        return handler(data, nodes, length, id);
    };
    parser.set_action_handler(name, fun);

    //parser.set_action_handler(name, fun);

}
EXPORT_API void WINAPI ParserSetHandler(DParser &parser, ParserHandler handler) {
    DParser::ParserActionFunction fun = [=](const DataType *data, const DNodes *nodes, size_t length, const char *id, bool &discard) {
        return handler(data, nodes, length, id);
    };
    parser.set_default_action_handler(fun);
}
EXPORT_API DataType WINAPI ParserParse(DParser &parser, const char *data) {
    parser.parse(data, data + strlen(data));
    return parser.user_data();
}
EXPORT_API const char *WINAPI NodesGetString(DNodes *node, int index) {
    return node[index].lexeme().c_str();
}
EXPORT_API int WINAPI NodesGetLine(DNodes *node, int index) {
    return node[index].line();
}
EXPORT_API int WINAPI NodesGetColumn(DNodes *node, int index) {
    return node[index].column();
}
EXPORT_API int WINAPI NodesSymbol(DNodes *node, int index) {
    return node[index].symbol()->type;
}
EXPORT_API const char *WINAPI NodesGetIdentifier(DNodes *node, int index) {
    return node[index].symbol()->identifier;
}
EXPORT_API const char *WINAPI NodesGetLexme(DNodes *node, int index) {
    return node[index].symbol()->lexeme;
}

#endif //GEDITOR_DLL_H
