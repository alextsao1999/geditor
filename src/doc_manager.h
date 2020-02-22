//
// Created by Alex on 2020/2/17.
//

#ifndef GEDITOR_DOC_MANAGER_H
#define GEDITOR_DOC_MANAGER_H

#include "document.h"
#include "table.h"
#include <codecvt>
#include <fstream>
typedef void (CALLBACK *Handler)(const char *, const char *);
class CallBackMsgHandler : public MessageHandler {
public:
    Handler m_onNotify = nullptr;
    Handler m_onResponse = nullptr;
    Handler m_onError = nullptr;
    void onNotify(string_ref method, value &params) override {
        if (m_onNotify) {
            auto p = params.dump();
            m_onNotify(method.c_str(), p.c_str());
        }
    }
    void onResponse(value &ID, value &result) override {
        auto id = ID.get<std::string>();
        auto p = result.dump();
        //printf("id:%s result -> %s\n", id.c_str(), p.c_str());
        printf("id:%s\n", id.c_str());
        if (m_onResponse) {
            m_onResponse(id.c_str(), p.c_str());
        }
    }
    void onError(value &ID, value &error) override {
        if (m_onError) {
            auto id = ID.dump();
            auto p = error.dump();
            m_onError(id.c_str(), p.c_str());
        }
    }
    void onRequest(string_ref method, value &params, value &ID) override {

    }
};
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
    CallBackMsgHandler m_handler;
    std::thread m_loop;
    std::vector<Document *> m_documents;
    std::vector<std::string> m_completionTrigger;
    std::vector<std::string> m_signatureTrigger;
    RenderManager *m_render;
    int m_current = 0;
public:
    explicit DocumentManager(RenderManager *render) : m_render(render) {
        //CreateLSP(R"(I:\lsp\ccls\cmake-build-release\ccls.exe)");
        openFile("C:/Users/Administrator/Desktop/compiler4e/runtime.c");

    }
    ~DocumentManager() {
        if (m_client) {
            m_client->Shutdown();
            m_client->Exit();
            m_loop.detach();
        }
    }

    void CreateLSP(string_ref path, string_ref cmd = nullptr) {
        //m_client = std::make_shared<LanguageClient>(R"(F:\LLVM\bin\clangd.exe)");
        //m_client = std::make_shared<LanguageClient>(R"(I:\lsp\ccls\cmake-build-release\ccls.exe)");
        m_client = std::make_shared<LanguageClient>(path.c_str(), (char *) cmd.c_str());
        m_loop = std::thread([&] { m_client->loop(m_handler); });

        string_ref ref = "file:///C:/Users/Administrator/Desktop/compiler4e/";
        m_client->Initialize(ref);
    }
    LanguageClient *getLanguageClient() { return m_client.get(); }
    void openFile(string_ref path) {
        m_current = m_documents.size();
        Document *doc = new FileDocument(this, path);
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
    DocumentManager *LSPManager() {
        if (m_client)
            return this;
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
                m_client->Completion(current()->getUri(), context.position(), ctx);
            }
        }
        for (auto &cmp : m_signatureTrigger) {
            if (cmp.front() == ch) {
                m_client->SignatureHelp(current()->getUri(), context.position());
            }
        }
    }
    void onHover(Position position) {
        m_client->Hover(current()->getUri(), position);
    }
    void onSignatureHelp(Position postion) {
        m_client->SignatureHelp(current()->getUri(), postion);
    }
};


#endif //GEDITOR_DOC_MANAGER_H
