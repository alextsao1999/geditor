//
// Created by 曹顺 on 2019/5/31.
//
#ifndef PARSE_E_FILE_TRANSLATOR_H
#define PARSE_E_FILE_TRANSLATOR_H

#include <ast.h>
#include <visitor.h>
#include "ECodeParser.h"

class Translator : Visitor {
private:
    ESub *current{};
    ECode *code{};
public:
    explicit Translator(ECode *code) : code(code) {}
    void enter_sub(ESub *enter) {
        current = enter;
    };
    EConst *find_const(Key key) {
        if (key.type != KeyType_Constant) {
            return nullptr;
        }
        for (auto & constant : code->constants) {
            if (constant.key.value == key.value) {
                return &constant;
            }
        }
        return nullptr;
    }
    ESub *find_sub(Key key) {
        for (auto & sub : code->subs) {
            if (sub.key.value == key.value) {
                return &sub;
            }
        }
        return nullptr;
    }
    EStruct *find_struct(Key key) {
        for (auto & i : code->structs) {
            if (i.key.value == key.value) {
                return &i;
            }
        }
        return nullptr;
    }

    /**
     * 查找局部变量或者参数
     * @param key
     * @return
     */
    EVar *find_local(Key key) {
        for (auto & local : current->locals) {
            if (local.key.value == key.value) {
                return &local;
            }
        }
        for (auto & param : current->params) {
            if (param.key.value == key.value) {
                return &param;
            }
        }
        return nullptr;
    }

    /**
     * 查找程序集变量
     * @param key
     * @return
     */
    EVar *find_program_var(Key key) {
        EModule *module = current->module;
        for (auto & var : module->vars) {
            if (var.key.value == key.value) {
                return &var;
            }
        }
        return nullptr;
    }
    PLIB_CONST_INFO find_lib_const(int lib_index, int member_index) {
        if (code->libraries.size() >= lib_index) {
            return nullptr;
        }
        return &code->libraries[lib_index].info->m_pLibConst[member_index];
    }

private:
    void visit(ASTProgram *node) override {
        for (auto &stmt : node->stmts) {
            if (stmt != nullptr) {
                stmt->accept(this);
            }
        }
    }
};


#endif //PARSE_E_FILE_TRANSLATOR_H
