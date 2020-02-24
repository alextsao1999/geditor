//
// Created by Alex on 2020/2/17.
//

#include "doc_manager.h"

FileDocument::FileDocument(DocumentManager *mgr, string_ref path, string_ref languageId) : Document(mgr) {
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
        str = &m_context.m_textBuffer.appendLine().content();

    } while (std::getline(is, *str));
    if (mgr->m_client)
        mgr->m_client->DidOpen(uri.str(), text, languageId);
    is.close();
    layout();
}

void FileDocument::onContentChange(EventContext &context, CommandType type, CommandData data) {
    if (m_manager->m_client) {
//        Range range;
//        range.start = context.position();
//        range.end = range.start;
//        range.end.character++;
        std::vector<TextDocumentContentChangeEvent> events;
        TextDocumentContentChangeEvent s;
        s.text = getContent();
//        s.range = range;
        events.push_back(s);
        m_manager->m_client->DidChange(getUri(), events, true);
    }
}
void NewDocument::onContentChange(EventContext &context, CommandType type, CommandData data) {
    if (m_manager->m_client) {
        std::vector<TextDocumentContentChangeEvent> events;
        TextDocumentContentChangeEvent s;
        s.text = getContent();
        events.push_back(s);
        m_manager->m_client->DidChange(getUri(), events, true);
    }

}

FileDocument::~FileDocument() {
    if (m_manager->m_client) {
        m_manager->m_client->DidClose(uri.str());
    }
}

void CallBackMsgHandler::onResponse(value &ID, value &result) {
    auto id = ID.get<RequestID>();
    auto content = result.dump();
    if (id == "initialize") {
        json &cap = result["capabilities"];
        if (cap.contains("completionProvider")) {
            cap["completionProvider"]["triggerCharacters"].get_to(m_mgr->m_completionTrigger);
        }
        if (cap.contains("signatureHelpProvider")) {
            cap["signatureHelpProvider"]["triggerCharacters"].get_to(m_mgr->m_signatureTrigger);
        }
    }
    printf("id:%s result -> %s\n\n", id.c_str(), content.c_str());
    //printf("id:%s\n", id.c_str());
    if (m_onResponse) {
        m_onResponse(id.c_str(), content.c_str());
    }
}

