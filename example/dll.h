//
// Created by Alex on 2020/1/21.
//

#ifndef GEDITOR_DLL_H
#define GEDITOR_DLL_H

#include "common.h"
#include "geditor.h"

#include <Windows.h>
#define EXPORT_API extern "C" __declspec(dllexport)
typedef int BOOL;
EXPORT_API GEditor *WINAPI CreateGeditor(HWND hWnd, int x, int y, int width, int height) {
    return GEditorBuilder::build(hWnd, x, y, width, height);
}
EXPORT_API void WINAPI DeleteGeditor(GEditor *editor) {
    delete editor;
}
EXPORT_API HWND WINAPI GEditorGetHWnd(GEditor *editor) { return editor->m_data->m_hwnd; }
EXPORT_API void WINAPI GEditorSetOnBlur(GEditor *editor, CallBack callback) {
    editor->m_data->current().m_onBlur = callback;
}
EXPORT_API void WINAPI GEditorSetStyle(GEditor *editor, int style_id, GColor color) {
    GStyle style;
    style.setColor(color);
    style.setTextSize(14);
    style.setTextEncoding(SkPaint::TextEncoding::kUTF16_TextEncoding);
    style.setAntiAlias(true);
    editor->m_data->current().m_context.m_styleManager.set(style_id, style);
}
EXPORT_API void WINAPI GEditorAddKeyword(GEditor *editor, int style, const char *keyword) {
    editor->m_data->current().m_context.keywords.emplace(AnsiToUnicode(keyword), style);
}
EXPORT_API void WINAPI GEditorSetColor(GEditor *editor, int style, GColor color) {
    editor->m_data->current().m_context.m_styleManager.get(style).setColor(color);
}
EXPORT_API void WINAPI GEditorRelayout(GEditor *editor) {
    editor->m_data->current().m_root.relayout();
}
EXPORT_API EventContext *WINAPI GEditorGetFocus(GEditor *editor) {
    return editor->m_data->current().m_context.m_caretManager.getEventContext();
}
EXPORT_API EventContext *WINAPI GEditorGetRoot(GEditor *editor) {
    return &editor->m_data->current().m_root;
}
EXPORT_API int WINAPI GeditorGetLineCount(GEditor *editor) {
    return editor->m_data->current().m_context.m_textBuffer.getLineCount();
}
EXPORT_API void WINAPI GeditorLineInsert(GEditor *editor, int line, const char *string) {
    auto viewer = editor->m_data->current().m_context.m_textBuffer.insertLine(line);
    if (string) {
        viewer.append(A2W(string));
    }
}
EXPORT_API int WINAPI GeditorLineAppend(GEditor *editor, const char *string) {
    LineViewer line = editor->m_data->current().m_context.m_textBuffer.appendLine();
    line.content().append(A2W(string));
    return line.m_line;
}
EXPORT_API void WINAPI GeditorLineDelete(GEditor *editor, int line) {
    editor->m_data->current().m_context.m_textBuffer.deleteLine(line);
}
EXPORT_API void WINAPI GeditorLineSetString(GEditor *editor, int line, const char *string) {
    LineViewer viewer = editor->m_data->current().m_context.m_textBuffer.getLine(line);
    viewer.content().assign(A2W(string));
}
EXPORT_API int WINAPI GeditorLineGetLength(GEditor *editor, int line) {
    auto &string = editor->m_data->current().m_context.m_textBuffer.getLine(line).content();
    return WideCharToMultiByte(0, 0, &string.front(), string.length(), 0, 0, 0, 0);
}
EXPORT_API void WINAPI GeditorLineGetString(GEditor *editor, int line, char *string, int length) {
    auto &content = editor->m_data->current().m_context.m_textBuffer.getLine(line).content();
    if (content.length() == 0) {
        return;
    }
    WideCharToMultiByte(0, 0, &content.front(), content.length(), string, length, 0, 0);
}
EXPORT_API Element *WINAPI GeditorAppendElement(GEditor *editor, Element *element) {
    return editor->m_data->current().append(element);
}
EXPORT_API EventContext *WINAPI EventContextCopy(EventContext *context) {
    return context->copy();
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
    sprintf(string, "%s", UnicodeToAnsi(context->tag().str).c_str());
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
    return context->isTail();
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
    context->getDocContext()->m_caretManager.update();
}
EXPORT_API EventContext *WINAPI EventContextFindPrev(EventContext *context, const char *tag) {
    return context->findPrev(A2W(tag));
}
EXPORT_API EventContext *WINAPI EventContextFindNext(EventContext *context, const char *tag) {
    return context->findNext(A2W(tag));
}
EXPORT_API EventContext *WINAPI EventContextFindInner(EventContext *context, const char *tag) {
    return context->findInnerFirst(A2W(tag));
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
EXPORT_API Element *WINAPI CreateTableElement(int row, int column, int top) {
    return new FastTable(row, column, top);
}
EXPORT_API Element *WINAPI CreateRowElement(int column, int height) {
    return new FastRow(column, height);
}
EXPORT_API void WINAPI RowSetColumnString(FastRow *row, int column, const char *string) {
    row->getColumn(column)->m_data.assign(A2W(string));
}
EXPORT_API void WINAPI RowSetHeader(FastRow *row, BOOL header) {
    row->setHeader(header);
}
EXPORT_API void WINAPI RowSetUndeleteable(FastRow *row, BOOL de) {
    row->setUndeleteable(de);
}
EXPORT_API void WINAPI RowSetColor(FastRow *row, GColor color) {
    row->setColor(color);
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
EXPORT_API int WINAPI RowGetColumnCount(FastRow *row) {
    return row->m_items.size();
}
EXPORT_API void WINAPI SubSetContent(SubElement *sub , int column, const char *name) {
    sub->content(column).assign(A2W(name));
}
EXPORT_API void WINAPI SubAddParam(SubElement *sub, const char *name, const char *type) {
    sub->addParam(A2W(name), A2W(type));
}
EXPORT_API void WINAPI SubAddLocal(SubElement *sub, const char *name, const char *type) {
    sub->addLocal(A2W(name), A2W(type));
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

#endif //GEDITOR_DLL_H
