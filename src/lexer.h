//
// Created by Alex on 2019/10/7.
//

#ifndef GEDITOR_LEXER_H
#define GEDITOR_LEXER_H

#include "text_buffer.h"
#include "paint_manager.h"
#include <set>
#define _HM_C _GT
#define CURRENT_CHAR

//当前位置
#define CURRENT_POS position
#define CURRENT_CHAR string[position]
#define CURRENT_TOKEN current
#define NEXT_CHAR string[position + 1]
#define NEXT() position++
#define TOKEN_START() {current.start = &CURRENT_CHAR;}
#define TOKEN_END(tok_type, end_style) {\
current.type = tok_type; \
current.style = end_style; \
current.length = int(&string[CURRENT_POS] - current.start); \
current.next = CURRENT_POS; \
}

class EventContext;
bool IsNumber(GChar ch);
bool IsAlpha(GChar ch);
bool IsCodeChar(GChar ch);
bool IsSpace(GChar ch);
enum TokenType {
    TokenNone,
    TokenSpace,
    TokenIdentifier,
    TokenNumber,
    TokenString,
    TokenOperator,
};

struct Token {
    int style{0};
    int type{TokenNone};
    const GChar *start{nullptr};
    int length{0};
    int next;
    const GChar *c_str() { return start; }
    size_t size() { return length * sizeof(GChar); }
};

static GString symbols = _GT("+-*/");
static std::map<GString, int> keywords = {
        {_GT("if"), StyleKeyword},
        {_GT("while"), StyleKeyword},
        {_GT("var"), StyleKeyword},
        {_GT("this"), StyleKeyword},
        {_GT("break"), StyleKeyword},
        {_GT("do"), StyleKeyword},
        {_GT("class"), StyleKeyword},
};

class Lexer {
    EventContext *context{};
    LineViewer viewer;
    const GChar *string{};
    int position = 0;
    int length = 0;
    Token current;
public:
    explicit Lexer() = default;
    virtual void enter(EventContext *context, int column);
    virtual bool has();
    virtual Token next();
private:
    void ParseSpace();
    void ParseIdentifier();
    void ParseString();
    void ParseNumber();
};

#endif //GEDITOR_LEXER_H
