//
// Created by Alex on 2020/4/25.
//

#ifndef PARSER_PARSERBUILDER_H
#define PARSER_PARSERBUILDER_H
#include <vector>
#include <memory>
#include <map>
#include <uri.h>
#include <Lexer.h>
template <typename char_t = char>
struct ParserBuilder {
    using string_t = std::basic_string<char_t>;
    class Rule;
    class Parser;
    using Value = nlohmann::json;
    using Lexer = Lexer<const char_t *>;
    using RulePtr = std::shared_ptr<Rule>;
    using ParserPtr = Parser *;
    using RuleVector = std::vector<RulePtr>;
    class Rule {
    public:
        virtual void error(Lexer &lexer, Value &value) {
            value["locate"] = {{"line",   lexer.line()},
                               {"column", lexer.column()},
                               {"length", lexer.token().length}};
        }
        virtual void locate(Lexer &lexer, Value &value) {
            value["locate"] = {{"line",   lexer.line()},
                               {"column", lexer.column()},
                               {"length", lexer.token().length}};
        }
        virtual bool parse(Lexer &lexer, Value &value) {
            return false;
        }
    };
    class RuleNumber : public Rule {
    public:
        bool parse(Lexer &lexer, Value &value) override {
            if (lexer.token() == TokenNumber) {
                Value literal;
                literal["type"] = "number";
                literal["value"] = lexer.token().str();
                locate(lexer, literal);
                value.push_back(literal);
                lexer.advance();
                return true;
            }
            return false;
        }
    };
    class RuleString : public Rule {
    public:
        bool parse(Lexer &lexer, Value &value) override {
            if (lexer.token() == TokenLiteral) {
                Value literal;
                literal["type"] = "literal";
                literal["value"] = lexer.token().str();
                locate(lexer, literal);
                value.push_back(literal);
                lexer.advance();
                return true;
            }
            return false;
        }
    };
    class RuleIdentifier : public Rule {
    public:
        bool parse(Lexer &lexer, Value &value) override {
            if (lexer.token() == TokenIdentifier) {
                Value literal;
                literal["type"] = "identifier";
                literal["value"] = lexer.token().str();
                locate(lexer, literal);
                value.push_back(literal);
                lexer.advance();
                return true;
            }
            return false;
        }
    };
    class RuleParser : public Rule {
    public:
        ParserPtr m_parser;
        RuleParser(const ParserPtr &mParser) : m_parser(mParser) {}
        bool parse(Lexer &lexer, Value &value) override;
    };
    class RuleMatch : public Rule {
    public:
        string_t m_pattern;
        RuleMatch(const char_t *pattern) : m_pattern(pattern) {}
        bool parse(Lexer &lexer, Value &value) override {
            if (lexer.token() == m_pattern.c_str()) {
                Value v;
                v["type"] = "match";
                v["value"] = m_pattern;
                locate(lexer, v);
                value.push_back(v);
                lexer.advance();
                return true;
            }
            return false;
        }
    };
    class RuleReduce : public Rule {
    public:
        std::string m_name;
        int m_reduce;
        RuleReduce(const char *mName, int mReduce) : m_name(mName), m_reduce(mReduce) {}
        bool parse(Lexer &lexer, Value &value) override {
            Value reduce = Value::array();
            for (int i = 0; i < m_reduce; ++i) {
                if (value.size() < 1) {
                    return false;
                }
                reduce.insert(reduce.begin(), value.back());
                value.pop_back();
            }
            value.push_back({{"type",  m_name}, {"value", reduce}});
            return true;
        }
    };
    class RuleRepeat : public Rule {
    public:
        ParserPtr m_parser;
        int m_from = 0, m_to = 0;
        RuleRepeat(ParserPtr mParser, int mFrom, int mTo) : m_parser(mParser), m_from(mFrom), m_to(mTo) {}
        bool parse(Lexer &lexer, Value &value) override;
    };
    class RuleOr : public Rule {
    public:
        std::vector<ParserPtr> m_parsers;
        RuleOr(std::vector<ParserPtr> prs) : m_parsers(std::move(prs)) {}
        bool parse(Lexer &lexer, Value &value) override;
    };
    class Parser {
    public:
        std::string m_name;
        RuleVector m_rules;
        Parser() = default;
        Parser(const std::string name) : m_name(name) {}
        void init(Lexer &lexer) {
            lexer.advance();
        }
        bool parse(Lexer &lexer, Value &value) {
            if (m_name.empty()) {
                for (auto &rule : m_rules) {
                    if (!rule->parse(lexer, value)) {
                        return false;
                    }
                }
                return true;
            }
            Value children;
            for (auto &rule : m_rules) {
                if (!rule->parse(lexer, children)) {
                    error(lexer, children);
                    return false;
                }
            }
            value.push_back({{"type", m_name}, {"value", children}});
            return true;
        }
        void error(Lexer &lexer, Value &value) {

        }
        ParserPtr id() {
            m_rules.emplace_back(new RuleIdentifier());
            return get_ptr();
        }
        ParserPtr number() {
            m_rules.emplace_back(new RuleNumber());
            return get_ptr();
        }
        ParserPtr string() {
            m_rules.emplace_back(new RuleString());
            return get_ptr();
        }
        ParserPtr add(Rule *rule) {
            m_rules.emplace_back(rule);
            return get_ptr();
        }
        ParserPtr cat(ParserPtr cat) {
            m_rules.emplace_back(new RuleParser(cat));
            return get_ptr();
        }
        ParserPtr match(const char_t *ptn) {
            m_rules.emplace_back(new RuleMatch(ptn));
            return get_ptr();
        }
        ParserPtr reduce(const char *type, int reduce) {
            m_rules.emplace_back(new RuleReduce(type, reduce));
            return get_ptr();
        }
        ParserPtr maybe(const std::vector<ParserPtr> &prs) {
            m_rules.emplace_back(new RuleOr(prs));
            return get_ptr();
        }
        ParserPtr repeat(ParserPtr psr, int from = 0, int to = -1) {
            m_rules.emplace_back(new RuleRepeat(psr, from, to));
            return get_ptr();
        }
        inline ParserPtr get_ptr() { return this; }
    };
    std::vector<std::unique_ptr<Parser>> m_rules;
    ParserPtr find(const char *name = "") {
        if (name != nullptr) {
            for (auto &item : m_rules) {
                if (item->m_name == name) {
                    return item;
                }
            }
        }
        return nullptr;
    }
    ParserPtr rule(const char *name = "") {
        m_rules.emplace_back(new Parser(name));
        return m_rules.back().get();
    }
    ParserPtr operator[](const char *name) {
        if (auto *ptr = find(name)) {
            return ptr;
        }
        return rule(name);
    }

};
template<typename char_t>
bool ParserBuilder<char_t>::RuleRepeat::parse(Lexer &lexer, ParserBuilder::Value &value) {
    int m_current = 0;
    if (m_parser->parse(lexer, value)) {
        m_current = 1;
        if (m_to < 0) {
            while (m_parser->parse(lexer, value)) {}
            return true;
        } else {
            while (m_current < m_to && m_parser->parse(lexer, value)) {
                m_current++;
            }
        }
        return true;
    }
    return m_current >= m_from;
}

template<typename char_t>
bool ParserBuilder<char_t>::RuleParser::parse(Lexer &lexer, ParserBuilder::Value &value) {
    return m_parser->parse(lexer, value);
}

template<typename char_t>
bool ParserBuilder<char_t>::RuleOr::parse(Lexer &lexer, ParserBuilder::Value &value) {
    for (auto &pr : m_parsers) {
        if (pr->parse(lexer, value)) {
            return true;
        }
    }
    return false;
}

#endif //PARSER_PARSERBUILDER_H
