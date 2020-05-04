//
// Created by Alex on 2020/5/3.
//

#ifndef GEDITOR_GRAMMERPARSER_H
#define GEDITOR_GRAMMERPARSER_H

#include <iostream>
#include "ParserBuilder.h"
#include "JsonWalker.h"
class GrammerParser {
public:
    using char_t = wchar_t;
    using Builder = ParserBuilder<char>;
    using GrammerBuilder = ParserBuilder<char_t>;
private:
    Builder m_builder;
    Builder::ParserPtr m_parser;

    GrammerBuilder m_grammer;
    JsonWalker<GrammerBuilder::ParserPtr> m_walker;
    std::map<std::string, GrammerBuilder::ParserPtr> m_rules;
public:
    GrammerParser() {
        auto *factor = m_builder.rule();
        auto *item = m_builder.rule("item")->repeat(factor);
        auto *postfix = m_builder.rule()->maybe({
            m_builder.rule()->match("*")->reduce("star", 2),
            m_builder.rule()->match("|")->cat(factor)->reduce("or", 3)
        });
        factor->maybe({
            m_builder.rule()->string(),
            m_builder.rule()->id(),
            m_builder.rule("compound")->match("(")->cat(item)->match(")"),
            m_builder.rule("reduce")->match("[")->string()->match(":")->number()->match("]")
        })->repeat(postfix);
        auto *rule = m_builder.rule("rule")->id()->match(":")->cat(item)->match(";");
        auto *ast = m_builder.rule("ast")->match("@")->id()->match(":")->cat(item)->match(";");
        auto *rule_factor = m_builder.rule()->maybe({rule, ast});
        m_walker.add_handler("reduce", [&](json &value) {
            std::string name = value[1]["value"];
            std::string number = value[3]["value"];
            return m_grammer.rule()->reduce(name.c_str(), std::stol(number));
        });
        m_walker.add_handler("identifier", [&](json &value) {
            std::string id = value;
            auto &item = m_rules[id];
            if (item == nullptr) {
                item = m_grammer.rule();
                return item;
            }
            return item;
        });
        m_walker.add_handler("literal", [&](json &value) {
            GrammerBuilder::string_t str = value;
            return m_grammer.rule()->match(str.c_str());
        });
        m_walker.add_handler("item", [&](json &value) {
            auto *rule = m_grammer.rule();
            for (auto &item : value) {
                std::string type = item["type"];
                if (type == "string") {
                    GrammerBuilder::string_t cont = item["value"];
                    rule->match(cont.c_str());
                } else {
                    rule->cat(m_walker.walk(item));
                }
            }
            return rule;
        });
        m_walker.add_handler("or", [&](json &value) {
            return m_grammer.rule()->maybe({m_walker.walk(value[0]), m_walker.walk(value[2])});
        });
        m_walker.add_handler("compound", [&](json &value) {
            return m_walker.walk(value[1]);
        });
        m_walker.add_handler("star", [&](json &value) {
            return m_grammer.rule()->repeat(m_walker.walk(value[0]));
        });
        m_walker.add_handler("rule", [&](json &value) {
            auto *rule = m_walker.walk(value[0]);
            rule->cat(m_walker.walk(value[2]));
            return rule;
        });
        m_walker.add_handler("ast", [&](json &value) {
            auto *rule = m_walker.walk(value[1]);
            rule->m_name = value[1]["value"].get<std::string>();
            rule->cat(m_walker.walk(value[3]));
            return rule;
        });
        m_walker.add_handler("grammer", [&](json &value) {
            auto *rule = m_grammer.rule();
            std::vector<ParserBuilder<char_t>::ParserPtr> ors;
            for (auto &item : value) {
                if (item["type"] == "rule" || item["type"] == "ast") {
                    ors.push_back(m_walker.walk(item));
                }
            }
            rule->maybe(ors);
            return rule;
        });
        m_rules["identifier"] = m_grammer.rule()->id();
        m_rules["number"] = m_grammer.rule()->number();
        m_rules["string"] = m_grammer.rule()->string();

        m_parser = m_builder.rule("grammer")->id()->match("{")->repeat(rule_factor)->match("}");
    }
    GrammerBuilder::ParserPtr compile(const char *grammer) {
        Builder::Lexer lexer;
        lexer.reset(grammer, strlen(grammer));
        Builder::Value value;
        m_parser->init(lexer);
        if (!m_parser->parse(lexer, value)) {
            return nullptr;
        }
        return m_walker(value[0]);
    }

};


#endif //GEDITOR_GRAMMERPARSER_H
