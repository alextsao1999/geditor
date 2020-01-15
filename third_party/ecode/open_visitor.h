//
// Created by Alex on 2020/1/15.
//

#ifndef GEDITOR_OPEN_VISITOR_H
#define GEDITOR_OPEN_VISITOR_H

#include "document.h"
#include "table.h"
#include <ECodeParser.h>
#include "visitor.h"
#include "ast.h"

std::wstring AnsiToUnicode(const char *str);
#define A2W(ansi) (AnsiToUnicode((const char *)(ansi)).c_str())
struct SubVisitor : Visitor {
    ECode *code;
    Document *document;
    ESub &current;
    EventContext context;
    LineViewer lineViewer;
    SubElement *element = nullptr;

    int lineIndex = 0;
    SubVisitor(ECode *code, Document *document, ESub &sub) : code(code), document(document), current(sub) {
        element = new SubElement();
        context = document->appendElement(element);

        auto *table = new TableElement(current.params.size() + 3, 4);
        table->getRow(0)->setColor(SkColorSetRGB(230, 237, 228));
        table->getItem(0, 0)->m_data.append(_GT("Function Name"));
        table->getItem(0, 1)->m_data.append(_GT("Type"));
        table->getItem(1, 0)->m_data.append(current.name.toUnicode().c_str());
        table->getItem(1, 1)->m_data.append(std::to_wstring(current.type));
        table->getRow(2)->setColor(SkColorSetRGB(230, 237, 228));
        table->getItem(2, 0)->m_data.append(_GT("Name"));
        table->getItem(2, 1)->m_data.append(_GT("Type"));
        table->replace(1, 3, new TableElement(2, 2));
        //table->replace(0, 3, new ButtonElement());
        for (int i = 0; i < current.params.size(); ++i) {
            auto *row = table->getRow(i + 3);
            row->getColumn(0)->m_data.append(current.params[i].name.toUnicode().c_str());
            row->getColumn(1)->m_data.append(std::to_wstring(current.params[i].type));
        }

        element->append(table);


        auto *vars = new TableElement(current.locals.size() + 1, 2);
        vars->getRow(0)->setColor(SkColorSetRGB(217, 227, 240));
        vars->getItem(0, 0)->m_data.append(_GT("Name"));
        vars->getItem(0, 0)->m_min = 100;
        vars->getItem(0, 1)->m_data.append(_GT("Value"));
        vars->getItem(0, 1)->m_min = 150;
        for (int i = 0; i < current.locals.size(); ++i) {
            auto *row = vars->getRow(i + 1);
            row->getColumn(0)->m_data.append(current.locals[i].name.toUnicode().c_str());
            row->getColumn(1)->m_data.append(std::to_wstring(current.locals[i].type));
        }
        element->append(vars);

    }

    void process(ASTFunCall *node) override {
        context.insertLine(lineIndex);
        lineViewer = context.getLineViewer(lineIndex);
        processing->current->append(new AutoLineElement());
        lineIndex++;
    }
    void process(ASTBlock *node) override {}

    void visit(ASTProgram *node) override {
        InnerElement container{};
        processing = &container;
        container.current = element;
        container.outer = nullptr;
        for (auto &stmt : node->stmts) {
            if (stmt != nullptr) {
                stmt->preprocess(this);
                stmt->accept(this);
            }
        }
    }
    void visit(ASTFunCall *node) override {
        if (node->lib >= 0) {
            lineViewer.append(A2W(code->libraries[node->lib].info->m_pBeginCmdInfo[node->key.value].m_szName));
            node->args->accept(this);
            return;
        }
        if (node->key.type == KeyType_Sub) {
            for (auto & sub : code->subs) {
                if (sub.key.value == node->key.value) {
                    lineViewer.append(sub.name.toUnicode().c_str());
                    node->args->accept(this);
                }
            }
        } else if (node->key.type == KeyType_DllFunc) {
            for (auto & dll : code->dlls) {
                if (dll.key.index == node->key.index) {
                    lineViewer.append(dll.name.toUnicode().c_str());
                    node->args->accept(this);
                }
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
        processing = container.enter(new SwitchElement(0), processing);
        //////////////////////////////////
        if (node->then_block) {
            InnerElement block;
            processing = block.enter(new CodeBlockElement(0), processing);
            node->condition->preprocess(this);
            lineViewer.append(_GT("if ("));
            node->condition->accept(this);
            lineViewer.append(_GT(")"));
            node->then_block->accept(this);
            processing = processing->outer;
        }
        if (node->else_block) {
            InnerElement block;
            processing = block.enter(new CodeBlockElement(0), processing);
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
                lineViewer.append(_GT("%d"), node->value.val_int);
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
        for (auto & constant : code->constants) {
            if (constant.key.value == node->key.value) {
                lineViewer.append(_GT("#"));
                lineViewer.append(constant.name.toUnicode().c_str());
            }
        }
    }
    void visit(ASTLibConstant *node) override {
        lineViewer.append(_GT("#"));
        lineViewer.append(A2W(code->libraries[node->index].info->m_pLibConst[node->member].m_szName));
    }
    void visit(ASTAddress *node) override {
        for (auto & sub : code->subs) {
            if (sub.key.value == node->key.value) {
                lineViewer.append(_GT("#"));
                lineViewer.append(sub.name.toUnicode().c_str());
            }
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
        for (auto & i : code->structs) {
            if (i.key.value == node->key.value) {
                for (auto & member : i.members) {
                    if (member.key.value == node->member.value) {
                        lineViewer.append(_GT("."));
                        lineViewer.append(member.name.toUnicode().c_str());
                        break;
                    }
                }
                break;
            }
        }
    }
    void visit(ASTVariable *node) override {
        if (node->key.type == KeyType_LocalOrParam) {
            for (auto & local : current.locals) {
                if (local.key.value == node->key.value) {
                    lineViewer.append(local.name.toUnicode().c_str());
                    return;
                }
            }
            for (auto & param : current.params) {
                if (param.key.value == node->key.value) {
                    lineViewer.append(param.name.toUnicode().c_str());
                    return;
                }
            }

        } else if (node->key.type == KeyType_ProgramVar) {
            EModule *module = current.module;
            for (auto & var : module->vars) {
                if (var.key.value == node->key.value) {
                    lineViewer.append(var.name.toUnicode().c_str());
                    return;
                }
            }
        }

    }
    void visit(ASTDot *node) override {
        node->var->accept(this);
        node->field->accept(this);
    }
    void visit(ASTJudge *node) override {

    }
    void visit(ASTLoop *node) override {
        InnerElement container;
        processing = container.enter(new SwitchElement(0), processing);
        //////////////////////////////////
        InnerElement block;
        processing = block.enter(new CodeBlockElement(0), processing);

        if (node->head) {
            node->head->preprocess(this);
            node->head->accept(this);
        }
        node->block->accept(this);
        if (node->tail) {
            node->tail->preprocess(this);
            node->tail->accept(this);
        }
        processing = processing->outer;
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
