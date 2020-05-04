//
// Created by Alex on 2019/10/7.
//

#ifndef GEDITOR_LEXER_H
#define GEDITOR_LEXER_H

#include "text_buffer.h"
#include "paint_manager.h"
#include <set>
#include <deque>

#define _HM_C _GT

//µ±«∞Œª÷√
#define HAS_CHAR (CURRENT_POS < length)
#define CURRENT_POS position
#define CURRENT_CHAR string[position]
#define CURRENT_TOKEN current
#define NEXT_CHAR string[position + 1]
#define NEXT() position++
#define TOKEN_START() {current.start = &CURRENT_CHAR;current.index = CURRENT_POS;}
#define TOKEN_END(tok_type, end_style) {\
current.type = tok_type; \
current.style = end_style; \
current.length = int(&string[CURRENT_POS] - current.start); \
current.next = CURRENT_POS; \
}

struct EventContext;
bool IsNumber(GChar ch);
bool IsAlpha(GChar ch);
bool IsCodeChar(GChar ch);
bool IsSpace(GChar ch);
enum TokenType {
    TokenNone,
    TokenEol,
    TokenSpace,
    TokenIdentifier,
    TokenNumber,
    TokenLiteral,
    TokenOperator,
};

struct Token {
    int style{0};
    int type{TokenEol};
    const GChar *start{nullptr};
    int length{0};
    int index{0};
    int next{0};
    const GChar *c_str() { return start; }
    size_t size() { return length * sizeof(GChar); }
    bool operator==(const GChar *str) {
        return gstrlen(str) == length && memcmp(start, str, length * sizeof(GChar)) == 0;
    }
    bool operator==(const int &rvalue) { return type == rvalue; }
    void dump() {
        printf("type: %d content: %ws length:%d\n", type, GString(start, length).c_str(), length);
    }
};

static GString symbols = _GT("+-*/");

class GLexer {
    EventContext *context{};
    LineViewer viewer;
    const GChar *string{};
    int position = 0;
    int length = 0;
    Token current;
    std::deque<Token> peeks;
public:
    explicit GLexer() = default;
    virtual void enter(EventContext *context);
    virtual bool has();
    virtual bool canNext();
    virtual Token next();
    Token peek(int num = 1);
private:
    void ParseSpace();
    void ParseIdentifier();
    void ParseString();
    void ParseNumber();
    void ParseNextToken();
};

#endif //GEDITOR_LEXER_H
