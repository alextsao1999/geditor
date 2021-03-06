//
// Created by Alex on 2020/2/17.
//

#include "doc_manager.h"

FileDocument::FileDocument(DocumentManager *mgr, string_ref path, string_ref languageId) : MarginDocument(mgr) {
    uri.from(path);
    std::wifstream is(path);
    std::string text;
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    GString *str = nullptr;
    do {
        if (str) {
            text.append(conv.to_bytes(*str));
            text.append("\r\n");
        }
        Container::append(new AutoLineElement());
        str = &buffer()->appendLine().content();
    } while (std::getline(is, *str));
    if (mgr->m_client)
        mgr->m_client->DidOpen(uri.str(), text, languageId);
    is.close();
    root().relayout();
}

void FileDocument::onContentChange(EventContext &context, CommandType type, CommandData data) {
    if (m_manager && m_manager->m_client) {
        std::vector<TextDocumentContentChangeEvent> events;
        TextDocumentContentChangeEvent append;
        if (type == CommandType::DeleteElement && 0) {
            int number = data.element->getLineNumber();
            Range range;
            range.start.line = context.line();
            range.start.character = 0;
            range.end.line = range.start.line + number;
            range.end.character = context.getLineViewer(number - 1).length();
            append.range = range;
            append.text = "";
            //append.rangeLength
            events.emplace_back(append);
        } else {
            append.text = getContent();
            events.push_back(append);
        }
        m_manager->m_client->DidChange(getUri(), events, true);
    }
}
void NewDocument::onContentChange(EventContext &context, CommandType type, CommandData data) {
    if (m_manager && m_manager->m_client) {
        std::vector<TextDocumentContentChangeEvent> events;
        TextDocumentContentChangeEvent s;
        s.text = getContent();
        events.push_back(s);
        m_manager->m_client->DidChange(getUri(), events, true);
    }

}

FileDocument::~FileDocument() {
    if (m_manager && m_manager->m_client) {
        m_manager->m_client->DidClose(uri.str());
    }
}

void CallBackMsgHandler::onResponse(value &ID, value &result) {
    auto id = ID.get<RequestID>();
    if (id == "initialize") {
        m_mgr->m_server = result["capabilities"];
    }
    if (m_onResponse) {
        m_onResponse(id.c_str(), result);
    }
    if (id == "textDocument/completion") {
        return;
    }
    //printf("------------------res------------ \n%s\n", result.dump().c_str());

}

