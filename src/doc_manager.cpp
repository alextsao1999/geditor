//
// Created by Alex on 2020/2/17.
//

#include "doc_manager.h"

FileDocument::FileDocument(DocumentManager *mgr, string_ref path) : Document(mgr) {
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
        mgr->m_client->DidOpen(uri.str(), text);
    is.close();
    layout();
}

void FileDocument::onContentChange(EventContext &context, CommandType type, CommandData data) {
    if (m_manager->m_client) {
        TextDocumentContentChangeEvent s;
        s.text = getContent();
        std::vector<TextDocumentContentChangeEvent> events;
        events.push_back(s);
        m_manager->m_client->DidChange(getUri(), events, true);
    }

}
