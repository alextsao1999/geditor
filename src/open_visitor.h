//
// Created by Alex on 2020/1/15.
//

#ifndef GEDITOR_OPEN_VISITOR_H
#define GEDITOR_OPEN_VISITOR_H

#include "document.h"
#include "table.h"
#include "ECodeParser.h"
#include "visitor.h"
#include "ast.h"

std::wstring AnsiToUnicode(const char *str);
std::string UnicodeToAnsi(const wchar_t *str);
#define A2W(ansi) (AnsiToUnicode((const char *)(ansi)).c_str())
#define W2A(unicode) (UnicodeToAnsi((const char *)(unicode)).c_str())
struct SubVisitor : Visitor {
    ECode *code;
    Document *document;
    Container<> *parent;
    LineViewer lineViewer;
    ESub *current = nullptr;
    SubVisitor(ECode *code, Document *document, Container<> *parent, ESub *sub) :
    code(code), document(document), parent(parent), current(sub) {}
    Container<> *create() {
        auto *subs = new SubElement();
        subs->content(0).append(current->name.toUnicode().c_str());
        subs->content(1).append(getType(current->type));
        if (!current->params.empty()) {
            for (auto &param : current->params) {
                subs->addParam(param.name.toUnicode().c_str(), getType(param.type).c_str());
            }
        }
        if (!current->locals.empty()) {
            for (auto &local : current->locals) {
                subs->addLocal(local.name.toUnicode().c_str(), getType(local.type).c_str());
            }
        }
        return subs;
    }
    std::wstring getType(Key key) {
        static std::map<int, const GChar *> maps = {
                {SDT_BYTE, _GT("byte")},
                {SDT_SHORT, _GT("short")},
                {SDT_INT, _GT("int")},
                {SDT_INT64, _GT("int64")},
                {SDT_FLOAT, _GT("float")},
                {SDT_DOUBLE, _GT("double")},
                {SDT_BOOL, _GT("bool")},
                {SDT_DATE_TIME, _GT("datetime")},
                {SDT_TEXT, _GT("string")},
                {SDT_BIN, _GT("binary")},
                {SDT_SUB_PTR, _GT("sub_ptr")},
                {SDT_STATMENT, _GT("statment")},
        };
        if (maps.count(key.value)) {
            return maps[key.value];
        }
        if (key.type == 0) {
            if (key.index == 48) {
                return _GT("Object");
            }
            if (key.index == 49) {
                return _GT("Varient");
            }
        }
        if (key.type == KeyType::KeyType_DataStruct) {
            auto *find = code->find<EStruct>(key.value);
            if (find) {
                return find->name.toUnicode();
            }
        }
        if (key.type == KeyType::KeyType_Module) {
            auto *find = code->find<EModule>(key.value);
            if (find) {
                return find->name.toUnicode();
            }
        }
        return std::to_wstring(key.value);
    }
    void process(ASTFunCall *node) override {
        lineViewer = document->m_context.m_textBuffer.appendLine();
        processing->current->append(ge_new(AutoLineElement));
    }
    void process(ASTVariable *node) override {
        lineViewer = document->m_context.m_textBuffer.appendLine();
        processing->current->append(ge_new(AutoLineElement));
    }
    void visit(ASTProgram *node) override {
        InnerElement container{};
        processing = &container;
        container.current = create();
        container.outer = nullptr;
        parent->append(container.current);
        for (auto &stmt : node->stmts) {
            if (stmt != nullptr) {
                stmt->preprocess(this);
                stmt->accept(this);
            }
        }
    }
    void visit(ASTFunCall *node) override {
        static std::map<std::string, std::wstring> binarys = {
                {"相加", _GT(" + ")},
                {"相乘", _GT(" * ")},
                {"赋值", _GT(" = ")},
                {"等于", _GT(" == ")},
                {"不等于", _GT(" != ")},
                {"大于", _GT(" > ")},
                {"小于", _GT(" < ")},
        };

        if (node->lib >= 0) {
            const char *str = (char *) code->libraries[node->lib].info->m_pBeginCmdInfo[node->key.value].m_szName;
            if (binarys.count(str)) {
                node->args->args[0]->accept(this);
                lineViewer.append(binarys[str].c_str());
                node->args->args[1]->accept(this);
                return;
            }
            lineViewer.append(A2W(code->libraries[node->lib].info->m_pBeginCmdInfo[node->key.value].m_szName));
            node->args->accept(this);
            return;
        }
        if (node->key.type == KeyType_Sub) {
            auto *sub = code->find<ESub>(node->key);
            if (sub) {
                lineViewer.append(sub->name.toUnicode().c_str());
                node->args->accept(this);
            }
        } else if (node->key.type == KeyType_DllFunc) {
            auto *dll = code->find<EDllSub>(node->key);
            if (dll) {
                lineViewer.append(dll->name.toUnicode().c_str());
                node->args->accept(this);
            }
        }
        if (node->comment.length) {
            lineViewer.append(_GT("'"));
            lineViewer.append(node->comment.toUnicode().c_str());
        }
    }
    void visit(ASTArgs *node) override {
        lineViewer.append(_GT(" ("));
        for (auto &arg : node->args) {
            if (arg != nullptr) {
                arg->accept(this);
                if (arg != node->args.back()) {
                    lineViewer.append(_GT(", "));
                }
            }
        }
        lineViewer.append(_GT(")"));
    }
    void visit(ASTBlock *node) override {
        for (auto &arg : node->element) {
            if (arg != nullptr) {
                arg->preprocess(this);
                arg->accept(this);
            }
        }
    }

    struct InnerElement {
        Container<> *current = nullptr;
        InnerElement *outer = nullptr;
        InnerElement *enter(Container<> *enter, InnerElement *out) {
            out->current->append(enter);
            current = enter;
            outer = out;
            return this;
        }
    };
    InnerElement *processing = nullptr;

    void visit(ASTIfStmt *node) override {
        InnerElement container;
        processing = container.enter(ge_new(SwitchElement, 0), processing);
        //////////////////////////////////
        if (node->then_block) {
            InnerElement block;
            processing = block.enter(ge_new(CodeBlockElement, 0), processing);
            node->condition->preprocess(this);
            lineViewer.append(_GT("if ("));
            node->condition->accept(this);
            lineViewer.append(_GT(")"));
            node->then_block->accept(this);
            processing = processing->outer;
        }
        if (node->else_block) {
            InnerElement block;
            processing = block.enter(ge_new(CodeBlockElement, 0), processing);
            node->else_block->accept(this);
            processing = processing->outer;
        }
        /////////////////////////////////
        processing = processing->outer;
    }
    void visit(ASTLiteral *node) override {
        switch (node->value.type) {
            case 0:
                lineViewer.append(_GT("null"));
                break;
            case 1:
                lineViewer.format(_GT("%d"), node->value.val_int);
                break;
            case 2:
                lineViewer.append(node->value.val_bool ? _GT("true") : _GT("false"));
                break;
            case 3:
                lineViewer.append(_GT("\""));
                lineViewer.append(node->value.val_string.toUnicode().c_str());
                lineViewer.append(_GT("\""));
                break;
            case 4:
                lineViewer.append(_GT("[time:"));
                lineViewer.format(_GT("%ld"), node->value.val_time);
                lineViewer.append(_GT("]"));
                break;
            case 5:
                lineViewer.format(_GT("%.0lf"), node->value.val_double);
                break;
            default:
                break;
        }

    }
    void visit(ASTConstant *node) override {
        auto *constant = code->find<EConst>(node->key);
        if (constant) {
            lineViewer.append(_GT("#"));
            lineViewer.append(constant->name.toUnicode().c_str());
        }
    }
    void visit(ASTLibConstant *node) override {
        lineViewer.append(_GT("#"));
        lineViewer.append(A2W(code->libraries[node->index].info->m_pLibConst[node->member].m_szName));
    }
    void visit(ASTAddress *node) override {
        auto *sub = code->find<ESub>(node->key);
        if (sub) {
            lineViewer.append(_GT("&"));
            lineViewer.append(sub->name.toUnicode().c_str());
        }
    }
    void visit(ASTSubscript *node) override {
        lineViewer.append(_GT("["));
        node->value->accept(this);
        lineViewer.append(_GT("]"));
    }
    void visit(ASTEnumConstant *node) override {
        lineViewer.append(_GT("..."));
    }
    void visit(ASTStructMember *node) override {
        auto *member = code->find<EVar>(node->member);
        if (member) {
            lineViewer.append(_GT("."));
            lineViewer.append(member->name.toUnicode().c_str());
        }
/*
        auto *stru = code->find<EStruct>(node->key);
        if (stru) {
            for (auto & member : stru->members) {
                if (member.key.value == node->member.value) {
                    lineViewer.append(_GT("."));
                    lineViewer.append(member.name.toUnicode().c_str());
                    break;
                }
            }

        }
*/
    }
    void visit(ASTVariable *node) override {
        auto *var = code->find<EVar>(node->key);
        if (var) {
            lineViewer.append(var->name.toUnicode().c_str());
            return;
        }
        return;

        if (node->key.type == KeyType_LocalOrParam) {
            for (auto & local : current->locals) {
                if (local.key.value == node->key.value) {
                    lineViewer.append(local.name.toUnicode().c_str());
                    return;
                }
            }
            for (auto & param : current->params) {
                if (param.key.value == node->key.value) {
                    lineViewer.append(param.name.toUnicode().c_str());
                    return;
                }
            }

        } else if (node->key.type == KeyType_ProgramVar) {
            auto *var = code->find<EVar>(node->key);
            if (var) {
                lineViewer.append(var->name.toUnicode().c_str());
                return;
            }
        }
    }
    void visit(ASTDot *node) override {
        node->var->accept(this);
        node->field->accept(this);
    }
    void visit(ASTJudge *node) override {
        InnerElement container;
        processing = container.enter(ge_new(SwitchElement, 0), processing);
        //////////////////////////////////
        for (int i = 0; i < node->conditions.size(); ++i) {
            InnerElement codeblock;
            processing = codeblock.enter(ge_new(CodeBlockElement, 0), processing);

            node->conditions[i]->preprocess(this);
            lineViewer.append(_GT("switch ("));
            node->conditions[i]->accept(this);
            lineViewer.append(_GT(")"));
            node->blocks[i]->accept(this);
            processing = processing->outer;
        }

        InnerElement codeblock;
        processing = codeblock.enter(ge_new(CodeBlockElement, 0), processing);
        node->default_block->accept(this);
        processing = processing->outer;
        /////////////////////////////////
        processing = processing->outer;

    }
    void visit(ASTLoop *node) override {
        InnerElement container;
        processing = container.enter(ge_new(LoopBlockElement, 0), processing);
        //////////////////////////////////
        if (node->head) {
            node->head->preprocess(this);
            node->head->accept(this);
        }
        node->block->accept(this);
        if (node->tail) {
            node->tail->preprocess(this);
            node->tail->accept(this);
        }
        /////////////////////////////////
        processing = processing->outer;

    }
    void visit(ASTBrace *node) override {
        lineViewer.append(_GT("{"));
        for (auto &arg : node->args->args) {
            if (arg != nullptr) {
                arg->accept(this);
                if (arg != node->args->args.back()) {
                    lineViewer.append(_GT(", "));
                }
            }
        }
        lineViewer.append(_GT("}"));
    }
};

#endif //GEDITOR_OPEN_VISITOR_H
