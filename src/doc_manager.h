//
// Created by Alex on 2020/2/17.
//

#ifndef GEDITOR_DOC_MANAGER_H
#define GEDITOR_DOC_MANAGER_H

#include "document.h"
#include "table.h"
#include "open_visitor.h"
#include "auto_complete.h"
#include <codecvt>
#include <fstream>
#include <regex>
#include <thread>
class DocumentManager;
typedef void (CALLBACK *Handler)(const char *, json &);
class CallBackMsgHandler : public MessageHandler {
public:
    Handler m_onNotify = nullptr;
    Handler m_onResponse = nullptr;
    Handler m_onError = nullptr;
    DocumentManager *m_mgr;
    CallBackMsgHandler(DocumentManager *mMgr) : m_mgr(mMgr) {}
    void onNotify(string_ref method, value &params) override {
        if (m_onNotify) {
            m_onNotify(method.c_str(), params);
        }
    }
    void onResponse(value &ID, value &result) override;
    void onError(value &ID, value &error) override {
        if (m_onError) {
            auto id = ID.dump();
            m_onError(id.c_str(), error);
        }
    }
    void onRequest(string_ref method, value &params, value &ID) override {}
};
using namespace lalr;
class NewDocument : public MarginDocument {
public:
    explicit NewDocument(DocumentManager *mgr) : MarginDocument(mgr) {
        m_grammer = new GrammarCompiler();
        const char* grammar =
                "Cpp {\n"
                "   %whitespace \"[ \\r\\n]*\";\n"
                "   items: items item [add($0,$1)] | item [create(items, $0)];\n"
                "   item:  block | identifier [$0())] | number [$0()] | \".\" [$0()];\n"
                "   block: "
                "      '{' items '}' [create(block, $0(), $1, $2())] | "
                "      '{' error [create(error, $0())] | "
                "      '{' '}' [create(block, $0(), $1())]; "
                "   number: \"[0-9]*\\.?[0-9]+\";\n"
                "   identifier: \"[A-Za-z\\x4e00-\\x9fa5_][A-Za-z0-9\\x4e00-\\x9fa5_]*\";\n"
                "}"
        ;
        m_grammer->compile(grammar, strlen(grammar) + grammar);
        Document::append(new AutoLineElement());
        buffer()->appendLine().append(_GT("dddd 0;"));
        buffer()->appendLine().append(_GT("return 0;"));
        Document::append(new AutoLineElement());
        buffer()->appendLine().append(_GT("asdfg 0;"));

/*
        auto *table = new TableElement(2, 5);
        table->getItem(0, 2)->m_radio = true;
        table->addRow(2);
        table->addRow(3);
        Document::append(table);
        Document::append(new AutoLineElement());
        buffer()->appendLine().append(_GT("return 0;"));
        buffer()->appendLine().append(_GT("return 0;"));
*/
        root().relayout();
/*
        auto *doc = new ClassElement();
        for (int i = 0; i < 2; ++i) {
            auto *sub = new SubElement();
            sub->content(0).assign(_GT("main"));
            sub->content(1).assign(_GT("int"));
            sub->addParam(_GT("argv"), _GT("char **"));
            sub->addLocal(_GT("temp"), _GT("int"));
            sub->append(new AutoLineElement());
            doc->append(sub);
            m_context.m_textBuffer.appendLine().append(_GT("return 0;"));
        }
        Document::append(doc);
*/
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
    std::string getContent() {
        std::string content;
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        for (auto &line : buffer()->iter()) {
            content.append(conv.to_bytes(line.content() + _GT("\r\n")));
        }
        return content;
    }
    void onContentChange(EventContext &context, CommandType type, CommandData data) override;

};
class EDocument : public MarginDocument {
public:
    explicit EDocument(DocumentManager *mgr, string_ref file) : MarginDocument(mgr) {
        FileBuffer buffer(file);
        ECodeParser parser(buffer);
        parser.Parse();
        for (auto &mod : parser.code.modules) {
            auto *module = new ModuleElement();
            module->name.append(mod.name.toUnicode());
            Container::append(module);
            for (auto &key : mod.include) {
                if (key.type == KeyType::KeyType_Sub) {
                    auto *sub = parser.code.find<ESub>(key);
                    if (sub) {
                        SubVisitor open(&parser.code, this, module, sub);
                        sub->ast->accept(&open);
                    }
                }
            }
        }
        buffer.free();
        root().relayout();
    }
};
class FileDocument : public MarginDocument {
private:
    URIForFile uri;
public:
    FileDocument(DocumentManager *mgr, string_ref path, string_ref languageId);
    ~FileDocument() override;
    string_ref getUri() override { return uri.str(); }
    std::string getContent() {
        std::string content;
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        for (auto &line : buffer()->iter()) {
            content += conv.to_bytes(line.content() + _GT("\r\n"));
        }
        return content;
    }
    void onContentChange(EventContext &context, CommandType type, CommandData data) override;

};
class DocumentManager {
public:
    //AutoComplete m_completion;
    std::shared_ptr<LanguageClient> m_client;
    ServerCapabilities m_server;
    CallBackMsgHandler m_handler;
    std::thread m_loop;
    std::vector<Document *> m_documents;
    RenderManager *m_render;
    int m_current = 0;
    std::map<GString, int> m_keywords = {
            {_GT("if"),    StyleKeywordFont},
            {_GT("while"), StyleKeywordFont},
            {_GT("var"),   StyleKeywordFont},
            {_GT("this"),  StyleKeywordFont},
            {_GT("break"), StyleKeywordFont},
            {_GT("do"),    StyleKeywordFont},
            {_GT("class"), StyleKeywordFont},
            {_GT("int"), StyleKeywordFont},
            {_GT("switch"), StyleKeywordFont},
            {_GT("true"), StyleKeywordFont},
            {_GT("false"), StyleKeywordFont},
            {_GT("null"), StyleKeywordFont},
            {_GT("return"), StyleKeywordFont},
    };
public:
    explicit DocumentManager(RenderManager *render) : m_render(render), m_handler(this) {
/*
        CreateLSP(R"(I:\lsp\ccls\cmake-build-release\ccls.exe)");
        string_ref ref = "file:///C:/Users/Administrator/Desktop/compiler4e/";
        m_client->Initialize(ref);
        openFile("C:/Users/Administrator/Desktop/compiler4e/runtime.c");
*/
        //open(new EDocument(this, R"(C:\Users\Administrator\Desktop\edit\f.e)"));
        openNew();
    }
    ~DocumentManager() {
        for (auto &doc : m_documents) {
            delete doc;
        }
        if (m_client) {
            m_client->Shutdown();
            m_client->Exit();
            m_loop.detach();
        }
    }
    void CreateLSP(string_ref path, string_ref cmd = nullptr) {
        //m_client = std::make_shared<LanguageClient>(R"(F:\LLVM\bin\clangd.exe)");
        //m_client = std::make_shared<LanguageClient>(R"(I:\lsp\ccls\cmake-build-release\ccls.exe)");
        m_client = std::make_shared<ProcessLanguageClient>(path.c_str(), (char *) cmd.c_str());
        m_loop = std::thread([&] { m_client->loop(m_handler); });
    }
    LanguageClient *getLanguageClient() { return m_client.get(); }
    int open(Document *document) {
        m_current = m_documents.size();
        m_documents.push_back(document);
        return m_current;
    }
    int openFile(string_ref path, string_ref languageId = "cpp") {
        m_current = m_documents.size();
        Document *doc = new FileDocument(this, path, languageId);
        m_documents.push_back(doc);
        return m_current;
    }
    int openNew() {
        m_current = m_documents.size();
        m_documents.push_back(new NewDocument(this));
        return m_current;
    }
    void close(int index) {
        auto *closed = m_documents[index];
        m_documents.erase(m_documents.begin() + index);
        delete closed;
        m_current = 0;
        if (m_documents.empty()) {
            openNew();
        }
    }
    void change(int index) {
        m_current = index;
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
        for (auto &cmp : m_server.completionTrigger) {
            if (cmp.front() == ch) {
                CompletionContext ctx;
                ctx.triggerKind = CompletionTriggerKind::TriggerCharacter;
                ctx.triggerCharacter = cmp;
                m_client->Completion(current()->getUri(), context.position(), ctx);
            }
        }
        for (auto &cmp : m_server.signatureHelpTrigger) {
            if (cmp.front() == ch) {
                m_client->SignatureHelp(current()->getUri(), context.position());
            }
        }
        if (&context == context.document()->caret()->getEventContext()) {
            CompletionContext ctx;
            auto pos = context.position(); // -1
            pos.character--;
            ctx.triggerKind = CompletionTriggerKind::TriggerTriggerForIncompleteCompletions;
            m_client->Completion(current()->getUri(), pos, ctx);
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
