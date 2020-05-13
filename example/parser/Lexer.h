//
// Created by Alex on 2020/4/26.
//

#ifndef GEDITOR_LEXER_H
#define GEDITOR_LEXER_H
#include <cstring>
#include <cctype>
#include <string>
enum TokenKind {
    TokenKindNone,
    TokenKindEnd,
    TokenKindIdentifier,
    TokenKindNumber,
    TokenKindLiteral,
    TokenKindOperator,
};
template<
        class iter_t = const char *,
        class char_t = typename std::iterator_traits<iter_t>::value_type,
        class traits_t = typename std::char_traits<char_t>,
        class allocator_t = typename std::allocator<char_t>>
class Lexer {
public:
    using string_t = std::basic_string<char_t, traits_t>;
    struct Token {
        TokenKind type = TokenKindNone;
        const char_t *string = nullptr;
        size_t length = 0;
        inline string_t str() const { return string_t(string, length); }
        inline bool operator==(const Token &rhs) const {
            if (type != rhs.type) {
                return false;
            }
            return length == rhs.length && memcmp(string, rhs.string, length) == 0;
        }

        inline bool operator==(const TokenKind &tt) const { return type == tt; }

        inline bool operator==(const char_t *tt) const {
            if (type == TokenKindEnd) {
                return false;
            }
            int check = 0;
            for (const char_t *position = string; *tt != '\0' && *position == *tt; ++tt) {
                check++;
            }
            return *tt == '\0' && length == check;
        }

        inline bool operator!=(const TokenKind &tt) const { return type != tt; }

        Token() = default;

        explicit Token(TokenKind type) : type(type) {}

        explicit Token(char_t chr) : type(TokenKindOperator) {}

        Token(TokenKind type, const char_t *start, const char_t *end) : type(type), string(start),
                                                                        length(end - start) {}
    };

private:
    const char_t *m_current = nullptr;
    const char_t *m_end = nullptr;
    const char_t *m_line_start = nullptr;
    char_t m_comment = '\\';
    int m_line = 0;
    Token m_token;
public:
    int line() { return m_line; }
    int column() { return m_current - m_line_start; }
    inline static bool is_identifier_first(char_t chr) {
        return isalpha(chr) || chr == char_t('_') || (chr >= char_t('\u4e00') && chr <= char_t('\u9fa5'));
    }
    inline static bool is_identifier_char(char_t chr) {
        return isalnum(chr) || chr == char_t('_') || (chr >= char_t('\u4e00') && chr <= char_t('\u9fa5'));
    }
    void reset(const char_t *start, size_t length) {
        m_current = start;
        m_line_start = m_current;
        m_end = start + length;
    }
    void skip() {
        while (has() && strchr(" \t\n\r", *m_current)) {
            if (*m_current == '\n') {
                m_line++;
                m_line_start = m_current;
            }
            m_current++;
        }
    }
    void skip_line() {
        while (has() && *(++m_current) != '\n');
        if (has()) {
            m_current++;
        }
    }
    bool match(const char_t *pattern) {
        auto *position = m_current;
        while (*pattern != '\0') {
            if (*pattern != *position) {
                break;
            }
            pattern++;
            position++;
        }
        if (*pattern == '\0') {
            m_current = position;
            return true;
        }
        return false;
    }
    void identifier() {
        if (is_identifier_first(*m_current)) {
            const char_t *start = m_current;
            do {
                m_current++;
            } while (has() && is_identifier_char(*m_current));
            m_token = Token(TokenKindIdentifier, start, m_current);
        }
    }
    void string() {
        const char_t *first = m_current++;
        while (has() && *m_current != *first) {
            m_current++;
            if (*m_current == '\\') {
                m_current = m_current + 2;
            }
        }
        m_token = Token(TokenKindLiteral, first + 1, m_current++);

    }
    void number() {
        const char_t *first = m_current;
        while (has() && isdigit(*m_current)) {
            m_current++;
        }
        m_token = Token(TokenKindNumber, first, m_current);
    }
    bool has() { return m_current != m_end; }
    bool good() const { return m_token.type != TokenKindEnd; }
    void advance() {
        skip();
        if (*m_current == m_comment) {
            skip_line();
        }
        if (m_current == m_end) {
            m_token = Token(TokenKindEnd);
            return;
        }
        if (*m_current == '\'' || *m_current == '"') {
            string();
            return;
        }
        if (isalpha(*m_current) || *m_current == '_') {
            identifier();
            return;
        }
        if (isdigit(*m_current)) {
            number();
            return;
        }

        m_token = Token(TokenKindOperator);
        m_token.string = m_current;
        m_token.length = 1;
        m_current++;
    }
    Token &token() {
        return m_token;
    }
    Token &next() {
        advance();
        return token();
    }
    Lexer() = default;
    Lexer(const char_t *start, const char_t *end) : m_current(start), m_end(end) {}
};

#endif //GEDITOR_LEXER_H
