//
// Created by 曹顺 on 2019/5/5.
//

#ifndef PARSE_E_FILE_VISITOR_H
#define PARSE_E_FILE_VISITOR_H

struct ASTNode;
struct ASTFunCall;
struct ASTProgram;
struct ASTArgs;
struct ASTBlock;
struct ASTIfStmt;
struct ASTLiteral;
struct ASTConstant;
struct ASTLibConstant;
struct ASTAddress;
struct ASTSubscript;
struct ASTEnumConstant;
struct ASTStructMember;
struct ASTVariable;
struct ASTDot;
struct ASTJudge;
struct ASTLoop;
struct ASTBrace;

struct Visitor {
    virtual void enter(ASTNode *node) {}
    virtual void leave(ASTNode *node) {}
    virtual void visit(ASTNode *node) {}
    virtual void visit(ASTFunCall *node) {}
    virtual void visit(ASTProgram *node) {}
    virtual void visit(ASTArgs *node) {}
    virtual void visit(ASTBlock *node) {}
    virtual void visit(ASTIfStmt *node) {}
    virtual void visit(ASTLiteral *node) {}
    virtual void visit(ASTConstant *node) {}
    virtual void visit(ASTLibConstant *node) {}
    virtual void visit(ASTAddress *node) {}
    virtual void visit(ASTSubscript *node) {}
    virtual void visit(ASTEnumConstant *node) {}
    virtual void visit(ASTStructMember *node) {}
    virtual void visit(ASTVariable *node) {}
    virtual void visit(ASTDot *node) {}
    virtual void visit(ASTJudge *node) {}
    virtual void visit(ASTLoop *node) {}
    virtual void visit(ASTBrace *node) {}


    virtual void process(ASTNode *node) {}
    virtual void process(ASTFunCall *node) {}
    virtual void process(ASTProgram *node) {}
    virtual void process(ASTArgs *node) {}
    virtual void process(ASTBlock *node) {}
    virtual void process(ASTIfStmt *node) {}
    virtual void process(ASTLiteral *node) {}
    virtual void process(ASTConstant *node) {}
    virtual void process(ASTLibConstant *node) {}
    virtual void process(ASTAddress *node) {}
    virtual void process(ASTSubscript *node) {}
    virtual void process(ASTEnumConstant *node) {}
    virtual void process(ASTStructMember *node) {}
    virtual void process(ASTVariable *node) {}
    virtual void process(ASTDot *node) {}
    virtual void process(ASTJudge *node) {}
    virtual void process(ASTLoop *node) {}
    virtual void process(ASTBrace *node) {}

};


#endif //PARSE_E_FILE_VISITOR_H
