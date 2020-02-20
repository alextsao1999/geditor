//
// Created by Alex on 2020/2/17.
//

#ifndef GEDITOR_DOC_MANAGER_H
#define GEDITOR_DOC_MANAGER_H

#include "document.h"
#include "table.h"
#include <codecvt>
#include <fstream>

class DocumentManager;
class NewDocument : public Document {
public:
    explicit NewDocument(DocumentManager *mgr) : Document(mgr) {
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
    URIForFile uri;
public:
    FileDocument(DocumentManager *mgr, string_ref path);
    string_ref getUri() override { return uri.str(); }
    std::string getContent() {
        std::string content;
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        for (auto &line : m_context.m_textBuffer.m_buffer) {
            content.append(conv.to_bytes(line.content + _GT("\r\n")));
        }
        return std::move(content);
    }
    void onContentChange(EventContext &context, CommandType type, CommandData data) override;

};
class DocumentManager {
public:
    AutoComplete m_completion;
    std::shared_ptr<LanguageClient> m_client;
    MapMessageHandler m_handler;
    std::thread m_loop;
    std::vector<Document *> m_documents;
    std::vector<std::string> m_completionTrigger;
    std::vector<std::string> m_signatureTrigger;
    RenderManager *m_render;
    int m_current = 0;
public:
    explicit DocumentManager(RenderManager *render) : m_render(render) {
        // m_client = std::make_shared<LanguageClient>(R"(F:\LLVM\bin\clangd.exe)");
        m_client = std::make_shared<LanguageClient>(R"(I:\lsp\ccls\cmake-build-release\ccls.exe)");
        string_ref ref = "file:///C:/Users/Administrator/Desktop/compiler4e/";
        m_handler.bindResponse(m_client->Initialize(ref), [=](value &value) {
            json &cap = value["capabilities"];
            if (cap.contains("completionProvider")) {
                cap["completionProvider"]["triggerCharacters"].get_to(m_completionTrigger);
            }
            if (cap.contains("signatureHelpProvider")) {
                cap["signatureHelpProvider"]["triggerCharacters"].get_to(m_signatureTrigger);
            }
            //capabilities["documentOnTypeFormattingProvider"]["firstTriggerCharacter"];

            m_client->Initialized();
        });
        m_loop = std::thread([&] {
            m_client->loop(m_handler);
        });
        openFile(R"(C:/Users/Administrator/Desktop/compiler4e/runtime.c)");
    }
    ~DocumentManager() {
        m_client->Exit();
        m_loop.detach();
    }
    LanguageClient *getLanguageClient() { return m_client.get(); }
    void openFile(DocumentUri uri) {
        m_current = m_documents.size();
        Document *doc = new FileDocument(this, uri);
        m_documents.push_back(doc);
    }
    void openNew() {
        m_current = m_documents.size();
        m_documents.push_back(new NewDocument(this));
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

    void onComplete(Position position, option<CompletionContext> context = {}) {
        m_client->Completion(current()->getUri(), position, context);
    }
    void onGoToDefinition(Position position) {
        m_client->GoToDefinition(current()->getUri(), position);
    }
    void onGoToDeclaration(Position position) {
        m_client->GoToDeclaration(current()->getUri(), position);
    }
    void onTrigger(EventContext &context, int ch) {
        for (auto &cmp : m_completionTrigger) {
            if (cmp.front() == ch) {
                CompletionContext ctx;
                ctx.triggerKind = CompletionTriggerKind::TriggerCharacter;
                auto id = m_client->Completion(current()->getUri(), context.position(), ctx);
                m_handler.bindResponse(id, [](value &value) {

                });
                return;
            }
        }
        for (auto &cmp : m_signatureTrigger) {
            if (cmp.front() == ch) {
                auto id = m_client->SignatureHelp(current()->getUri(), context.position());
                m_handler.bindResponse(id, [](value &value) {

                });
                return;
            }
        }
    }
    void onHover(Position position) {
        int id = m_client->Hover(current()->getUri(), position);
        m_handler.bindResponse(id, [] (value &value) {

        });
    }
    void onSignatureHelp(Position postion) {
        m_client->SignatureHelp(current()->getUri(), postion);
    }
};


#endif //GEDITOR_DOC_MANAGER_H
