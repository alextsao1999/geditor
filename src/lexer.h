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

//当前位置
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
    TokenString,
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
static std::map<GString, int> keywords = {
        {_GT("if"),    StyleKeywordFont},
        {_GT("while"), StyleKeywordFont},
        {_GT("var"),   StyleKeywordFont},
        {_GT("this"),  StyleKeywordFont},
        {_GT("break"), StyleKeywordFont},
        {_GT("do"),    StyleKeywordFont},
        {_GT("class"), StyleKeywordFont},
};

class Lexer {
    EventContext *context{};
    LineViewer viewer;
    const GChar *string{};
    int position = 0;
    int length = 0;
    Token current;
    std::deque<Token> peeks;
public:
    explicit Lexer() = default;
    virtual void enter(EventContext *context, int column);
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
