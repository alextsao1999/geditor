//
// Created by Alex on 2019/10/7.
//

#include "lexer.h"
#include "document.h"
#include "doc_manager.h"
bool IsSpace(GChar ch){
    return ch == _HM_C(' ') || ch == _HM_C('\r');
}
bool IsNumber(GChar ch) {
    return ch >= _HM_C('0') && ch <= _HM_C('9');
}
bool IsAlpha(GChar ch) {
    return (ch >= _HM_C('a') && ch <= _HM_C('z')) || (ch >= _HM_C('A') && ch <= _HM_C('Z'));
}
bool IsCodeChar(GChar ch) {
    return IsAlpha(ch) || ch == _HM_C('_') || (ch >= _GT('\u4E00') && ch <= _GT('\u9FA5'));
}

void GLexer::enter(EventContext *ctx) {
    context = ctx;
    viewer = context->getLineViewer();
    string = viewer.c_str();
    length = viewer.length();
    position = 0;
    peeks.clear();
}

bool GLexer::has() {
    return HAS_CHAR || !peeks.empty();
}

void GLexer::ParseSpace() {
    TOKEN_START();
    do {
        NEXT();
    } while (IsSpace(CURRENT_CHAR));
    TOKEN_END(TokenSpace, StyleDeafaultFont);
}

void GLexer::ParseIdentifier() {
    TOKEN_START();
    do {
        NEXT();
    } while (IsCodeChar(CURRENT_CHAR) || IsNumber(CURRENT_CHAR));
    TOKEN_END(TokenIdentifier, StyleDeafaultFont);
    GString identifier(current.start, current.length);
    if (auto *mgr = context->document()->getDocumentManager()) {
        current.style = mgr->m_keywords.count(identifier) ?
                        mgr->m_keywords[identifier] : StyleDeafaultFont;

    }
}

void GLexer::ParseString() {
    TOKEN_START();
    GChar first = CURRENT_CHAR;
    NEXT();
    while (CURRENT_CHAR != first) {
        NEXT();
        if (!HAS_CHAR) {
            TOKEN_END(TokenLiteral, StyleErrorFont);
            return;
        }
    }
    NEXT();
    TOKEN_END(TokenLiteral, StyleStringFont);
}

void GLexer::ParseNumber() {
    TOKEN_START();
    do {
        NEXT();
    } while (IsNumber(CURRENT_CHAR) || CURRENT_CHAR == _HM_C('.'));
    TOKEN_END(TokenNumber, StyleNumberFont);
}

Token GLexer::next() {
    if (peeks.empty()) {
        ParseNextToken();
    } else {
        CURRENT_TOKEN = peeks.front();
        peeks.pop_front();
    }
    return CURRENT_TOKEN;
}

Token GLexer::peek(int num) {
    while (peeks.size() < num) {
        ParseNextToken();
        if (CURRENT_TOKEN == TokenEol) {
            return CURRENT_TOKEN;
        }
        peeks.push_back(CURRENT_TOKEN);
    }
    return peeks[num - 1];
}

void GLexer::ParseNextToken() {
    if (!has()) {
        CURRENT_TOKEN = Token();
        current.type = TokenEol;
        return;
    }
    switch (CURRENT_CHAR) {
        case _HM_C(';'):case _HM_C('('):case _HM_C(')'):case _HM_C('['):
        case _HM_C(']'):case _HM_C('{'):case _HM_C('}'):case _HM_C('.'):
        case _HM_C('@'):case _HM_C(':'):case _HM_C('!'):case _HM_C('|'):
        case _HM_C('&'):case _HM_C('+'):case _HM_C('>'):case _HM_C('<'):
        case _HM_C('='):case _HM_C('%'):case _HM_C('-'):case _HM_C('*'):
        case _HM_C(','):case _HM_C('#'):
        case _HM_C('/'):
        TOKEN_START();
            NEXT();
            if (NEXT_CHAR == _HM_C('/') || NEXT_CHAR == _HM_C('*')) {
                NEXT();
            }
            TOKEN_END(TokenOperator, StyleOperatorFont);
            break;
        default:
            if (IsCodeChar(CURRENT_CHAR)) {
                ParseIdentifier();
            } else if (IsSpace(CURRENT_CHAR)) {
                ParseSpace();
            } else if (IsNumber(CURRENT_CHAR)) {
                ParseNumber();
            } else if (CURRENT_CHAR == _HM_C('"') || CURRENT_CHAR == _HM_C('\'')) {
                ParseString();
            } else {
                NEXT();
            }
    }
}

bool GLexer::canNext() {
    return CURRENT_POS < length - 1;
}

