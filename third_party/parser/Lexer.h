//
// Created by Alex on 2020/4/26.
//

#ifndef GEDITOR_LEXER_H
#define GEDITOR_LEXER_H
#include <cstring>
#include <cctype>
#include <string>
enum TokenType {
    TokenNone,
    TokenEnd,
    TokenIdentifier,
    TokenNumber,
    TokenLiteral,
    TokenOperator,
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
        TokenType type = TokenNone;
        const char_t *string = nullptr;
        size_t length = 0;
        inline string_t str() const { return string_t(string, length); }
        inline bool operator==(const Token &rhs) const {
            if (type != rhs.type) {
                return false;
            }
            return length == rhs.length && memcmp(string, rhs.string, length) == 0;
        }

        inline bool operator==(const TokenType &tt) const { return type == tt; }

        inline bool operator==(const char_t *tt) const {
            if (type == TokenEnd) {
                return false;
            }
            int check = 0;
            for (const char_t *position = string; *tt != '\0' && *position == *tt; ++tt) {
                check++;
            }
            return *tt == '\0' && length == check;
        }

        inline bool operator!=(const TokenType &tt) const { return type != tt; }

        Token() = default;

        explicit Token(TokenType type) : type(type) {}

        explicit Token(char_t chr) : type(TokenOperator) {}

        Token(TokenType type, const char_t *start, const char_t *end) : type(type), string(start),
                                                                        length(end - start) {}
    };

private:
    const char_t *m_current = nullptr;
    const char_t *m_end = nullptr;
    const char_t *m_line_start = nullptr;
    int m_line = 0;
    Token m_token;
public:
    int line() { return m_line; }
    int column() { return m_current - m_line_start; }
    void reset(const char_t *start, size_t length) {
        m_current = start;
        m_line_start = m_current;
        m_end = start + length;
    }
    void skip() {
        while (strchr(" \t\n\r", *m_current)) {
            if (*m_current == '\n') {
                m_line++;
                m_line_start = m_current;
            }
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
        if (isalpha(*m_current) || *m_current == '_') {
            const char_t *start = m_current;
            do {
                m_current++;
            } while (has() && isalnum(*m_current));
            m_token = Token(TokenIdentifier, start, m_current);
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
        m_token = Token(TokenLiteral, first + 1, m_current++);

    }
    void number() {
        const char_t *first = m_current;
        while (has() && isdigit(*m_current)) {
            m_current++;
        }
        m_token = Token(TokenNumber, first, m_current);
    }
    bool has() { return m_current != m_end; }
    bool good() const { return m_token.type != TokenEnd; }
    void advance() {
        if (m_current == m_end) {
            m_token = Token(TokenType::TokenEnd);
            return;
        }
        skip();
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

        m_token = Token(TokenType::TokenOperator);
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
