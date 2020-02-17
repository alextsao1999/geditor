//
// Created by Alex on 2020/2/17.
//

#ifndef GEDITOR_DOC_MANAGER_H
#define GEDITOR_DOC_MANAGER_H

#include "document.h"
#include "table.h"
#include <codecvt>
#include <fstream>
class NewDocument : public Document {
public:
    explicit NewDocument(RenderManager *renderManager) : Document(renderManager) {
        auto *table = new SubElement();
        table->addParam(L"test", L"aaaa");
        table->addLocal(L"test", L"aaaa");
        table->append(new AutoLineElement());
        Document::append(table);
        m_context.m_textBuffer.appendLine();
        layout();
    }
    void Open () {
        /*
        FileBuffer buffer(R"(C:\Users\Administrator\Desktop\edit\k.e)");
        ECodeParser parser(buffer);
        parser.Parse();
        int count = 0;
        for (auto &sub : parser.code.subs) {
            count++;
            SubVisitor open(&parser.code, &m_data->current(), sub);
            sub.ast->accept(&open);
        }
*/

    }

};
class FileDocument : public Document {
private:
    std::string uri;
    LanguageClient &m_client;
public:
    FileDocument(RenderManager *render, LanguageClient &client, string_ref path) : Document(render), m_client(client) {
        uri = "file:///C%3A/Users/Administrator/Desktop/compiler4e/runtime.c";
/*
        uri = "file:///";
        uri.append(path);
*/
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
        client.DidOpen(uri, text);
        is.close();
        layout();
    }
    string_ref getUri() override { return uri; }
    std::string getContent() {
        std::string content;
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        for (auto &line : m_context.m_textBuffer.m_buffer) {
            content.append(conv.to_bytes(line.content));
            content.append("\r\n");
        }
        return std::move(content);
    }
    LanguageClient *getLanguageClient() override {
        return &m_client;
    }
    void uploadContent() override {
        TextDocumentContentChangeEvent s;
        std::vector<TextDocumentContentChangeEvent> events;
        s.text = getContent();
        events.push_back(s);
        m_client.DidChange(getUri(), events, true);

    }
};
class DocumentManager {
private:
    LanguageClient m_client;
    MapMessageHandler m_handler;
    std::thread m_loop;
    std::vector<Document *> m_documents;
    RenderManager *m_render;
    int m_current = 0;
public:
    explicit DocumentManager(RenderManager *render) :
    m_render(render),
    m_client(R"(I:\lsp\ccls\cmake-build-release\ccls.exe)") {
        m_documents.push_back(new NewDocument(m_render));
        string_ref ref = "file:///C%3A/Users/Administrator/Desktop/compiler4e/";
        auto id = m_client.Initialize(ref);
        m_handler.bindResponse(id, [=](value &value) {
            m_client.Initialized();
        });
        m_loop = std::thread([&] {
            m_client.loop(m_handler);
        });
    }
    ~DocumentManager() {
        m_client.Exit();
        m_loop.detach();
    }
    void open(DocumentUri uri) noexcept {
        m_current = m_documents.size();
        Document *doc = new FileDocument(m_render, m_client, uri);
        m_documents.push_back(doc);
    }
    Document *current() {
        return m_documents[m_current];
    }
    Document *getDocument(DocumentUri uri) {
        for (auto &document : m_documents) {
            if (document->getUri() == uri) {
                return document;
            }
        }
        return nullptr;
    }

};


#endif //GEDITOR_DOC_MANAGER_H
