//
// Created by Alex on 2020/4/26.
//

#include <ParserBuilder.h>
#include <iostream>
#include <functional>
#include <JsonWalker.h>
#include "GrammerParser.h"
int run(json &json) {
    auto &type = json["type"];
    if (type == "binary") {
        auto &children = json["children"];
        auto iter = children.begin();
        auto end = children.begin();
        int res = run(*iter);
        while (++iter != end) {
            if ((*iter)["type"] == "match") {
                std::string op = (*iter)["value"];
                if (op == "+") {

                }
            }
        }
    }
    if (type == "") {

    }
    return 0;
}
void parsertest() {
    using Builder = ParserBuilder<char>;
    Builder::Lexer lexer;
    Builder builder;

    auto *factor = builder.rule();
    auto *term = builder.rule()
            ->cat(factor)
            ->repeat(
                    builder.rule()
                            ->maybe({builder.rule()->match("*"), builder.rule()->match("/")})
                            ->cat(factor)->reduce("binary", 3), 0, -1);

    auto *expr = builder.rule()
            ->cat(term)
            ->repeat(
                    builder.rule()
                            ->maybe({builder.rule()->match("+"), builder.rule()->match("-")})
                            ->cat(term)->reduce("binary", 3), 0, -1);

    factor->maybe({
                          builder.rule()->id(),
                          builder.rule()->number(),
                          builder.rule("compound")->match("(")->cat(expr)->match(")")});

    const char *start = "(30 + 40) * 2 +5";
    Builder::Value value;
    lexer.reset(start, strlen(start));
    expr->init(lexer);
    expr->parse(lexer, value);
    //printf("res:%d\n", run(value));
    JsonWalker<int> jw;
    jw["number"] = [&](json &value) {
        std::string number = value;
        return std::stol(number);
    };
    jw["compound"] = [&](json &value) {
        return jw(value[1]);
    };
    jw["binary"] = [&](json &value) {
        int num1 = jw(value[0]);
        int num2 = jw(value[2]);
        std::string op = value[1]["value"];
        if (op == "+") {
            return num1 + num2;
        }
        if (op == "-") {
            return num1 - num2;
        }
        if (op == "*") {
            return num1 * num2;
        }
        if (op == "/") {
            return num1 / num2;
        }
        return 0;
    };
    std::cout << value.dump();
    std::cout << "value:" << jw.walk(value[0]);

}
void parsebnf() {
    using Builder = ParserBuilder<char>;
    Builder::Lexer lexer;
    Builder builder;
    auto *factor = builder.rule();
    auto *item = builder.rule("item")->repeat(factor);
    auto *postfix = builder.rule()->maybe({
        builder.rule()->match("*")->reduce("star", 2),
        builder.rule()->match("|")->cat(factor)->reduce("or", 3)
    });
    factor->maybe({
                          builder.rule()->string(),
                          builder.rule()->id(),
                          builder.rule("compound")->match("(")->cat(item)->match(")"),
                          builder.rule("reduce")->match("[")->string()->match(":")->number()->match("]")
                  })->repeat(postfix);
    auto *rule = builder.rule("rule")->id()->match(":")->cat(item)->match(";");
    auto *ast = builder.rule("ast")->match("@")->id()->match(":")->cat(item)->match(";");
    auto *rule_factor = builder.rule()->maybe({rule, ast});
    auto *grammer = builder.rule("grammer")->id()->match("{")->repeat(rule_factor)->match("}");
    const char *start = "Cpp {"
                        "expr: term ( '+' | '-' term ['binary':3] )*;"
                        "term: factor ( '*' | '/' factor ['binary':3] )*;"
                        "factor: identifier | number | ( '(' expr ')' );"
                        "}";
    Builder::Value value;
    lexer.reset(start, strlen(start));
    grammer->init(lexer);
    grammer->parse(lexer, value);

    Builder bnf;
    std::map<std::string, Builder::ParserPtr> rules;
    rules["identifier"] = bnf.rule()->id();
    rules["number"] = bnf.rule()->number();
    rules["string"] = bnf.rule()->string();
    JsonWalker<Builder::ParserPtr> jw;
    jw.add_handler("reduce", [&](json &value) -> Builder::ParserPtr {
        //auto *rule = jw.walk(value[0]);
        std::string name = value[1]["value"];
        std::string number = value[3]["value"];
        return bnf.rule()->reduce(name.c_str(), std::stol(number));
    });
    jw.add_handler("identifier", [&](json &value) -> Builder::ParserPtr {
        std::string id = value;
        auto &item = rules[id];
        if (item == nullptr) {
            item = bnf.rule();
            return item;
        }
        return item;
    });
    jw.add_handler("literal", [&](json &value) -> Builder::ParserPtr {
        std::string str = value;
        return bnf.rule()->match(str.c_str());
    });
    jw.add_handler("item", [&](json &value) -> Builder::ParserPtr {
        auto *rule = bnf.rule();
        for (auto &item : value) {
            std::string type = item["type"];
            if (type == "string") {
                std::string cont = item["value"];
                rule->match(cont.c_str());
            } else {
                rule->cat(jw.walk(item));
            }

        }
        return rule;
    });
    jw.add_handler("or", [&](json &value) -> Builder::ParserPtr {
        return bnf.rule()->maybe({jw.walk(value[0]), jw.walk(value[2])});
    });
    jw.add_handler("compound", [&](json &value) -> Builder::ParserPtr {
        return jw.walk(value[1]);
    });
    jw.add_handler("star", [&](json &value) -> Builder::ParserPtr {
        return bnf.rule()->repeat(jw.walk(value[0]));
    });
    jw.add_handler("rule", [&](json &value) -> Builder::ParserPtr {
        auto *rule = jw.walk(value[0]);
        rule->cat(jw.walk(value[2]));
        return rule;
    });
    jw.add_handler("ast", [&](json &value) -> Builder::ParserPtr {
        auto *rule = jw.walk(value[1]);
        rule->m_name = value[1]["value"].get<std::string>();
        rule->cat(jw.walk(value[3]));
        return rule;
    });
    jw.add_handler("grammer", [&](json &value) -> Builder::ParserPtr {
        auto *rule = bnf.rule();
        std::vector<Builder::ParserPtr> ors;
        for (auto &item : value) {
            if (item["type"] == "rule" || item["type"] == "ast") {
                ors.push_back(jw.walk(item));
            }
        }
        rule->maybe(ors);
        return rule;
    });
    //std::cout << value.dump(1);
    jw.walk(value[0]);
    auto *grammer_parser = rules["expr"];
    //grammer_parser->dump();
    const char *expr = "1+1*3+2";
    Builder::Value result;
    lexer.reset(expr, strlen(expr));
    grammer_parser->init(lexer);
    grammer_parser->parse(lexer, result);
    std::cout << result.dump(1);

}
int main() {
    //parsertest();
    //parsebnf();
    GrammerParser grammer;
    const char *start = "Cpp {"
                        "program: expr*;"
                        "expr: term ( '+' | '-' term ['binary':3] )*;"
                        "term: factor ( '*' | '/' factor ['binary':3] )*;"
                        "factor: primary;"
                        "primary: identifier | number | ( '(' expr ')' ['compound':3] )"
                        "postfix: '(' arg_list ')' ['call_postfix':4];"
                        "@arg_list: factor (',' factor)*"
                        "}";
    auto *parser = grammer.compile(start);
    if (!parser) {
        return 0;
    }
    GrammerParser::GrammerBuilder::Lexer lexer;
    auto *source = L"1+2 2+4";
    lexer.reset(source, wcslen(source));
    GrammerParser::Builder::Value value;
    parser->init(lexer);
    parser->parse(lexer, value);

    std::cout << value.dump(1);
    return 0;
}