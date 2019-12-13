#include <text_buffer.h>
#include <lexer.h>
#include <document.h>
#include "SkCanvas.h"
#include "SkGraphics.h"
//#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
//#include "SkStream.h"
#include "SkString.h"
#include "SkTemplates.h"
#include "SkSurface.h"
#include "iostream"
#include "SkRect.h"
#include <table.h>
#include <stdio.h>
#include <stdarg.h>
#include <lalr/GrammarCompiler.hpp>
#include <lalr/Parser.ipp>
#include <utility>
#include <string.h>

using namespace std;
using namespace lalr;

void lalr_calculator_example()
{
    const char *calculator_grammar =
            "calculator { \n"
            "   %left '+' '-'; \n"
            "   %left '*' '/'; \n"
            "   %none integer; \n"
            "   %whitespace \"[ \t\r\n]*\"; \n"
            "   expr: \n"
            "      expr '+' expr [add] | \n"
            "      expr '-' expr [subtract] | \n"
            "      expr '*' expr [multiply] | \n"
            "      expr '/' expr [divide] | \n"
            "      '(' expr ')' [compound] | \n"
            "      integer [string] \n"
            "   ; \n"
            "   integer: \"[0-9]+\"; \n"
            "} \n"
            "";
    GrammarCompiler compiler;
    compiler.compile( calculator_grammar, calculator_grammar + strlen(calculator_grammar) );
    Parser<const char*, int> parser( compiler.parser_state_machine() );
    parser.parser_action_handlers()
            ( "add", [] ( const int* data, const ParserNode<>* nodes, size_t length )
              {
                  return data[0] + data[2];
              }
            )
            ( "subtract", [] ( const int* data, const ParserNode<>* nodes, size_t length )
              {
                  return data[0] - data[2];
              }
            )
            ( "multiply", [] ( const int* data, const ParserNode<>* nodes, size_t length )
              {
                  return data[0] * data[2];
              }
            )
            ( "divide", [] ( const int* data, const ParserNode<>* nodes, size_t length )
              {
                  return data[0] / data[2];
              }
            )
            ( "compound", [] ( const int* data, const ParserNode<>* nodes, size_t length )
              {
                  return data[1];
              }
            )
            ( "integer", [] ( const int* data, const ParserNode<>* nodes, size_t length )
              {
                  return ::atoi( nodes[0].lexeme().c_str() );
              }
            )
            ;

    const char* input = "1 + 2 * 2";
    parser.parse( input, input + strlen(input) );
    printf( "%d  ", parser.user_data() );
    //printf( "%d", parser.user_data() );
}

struct ASTBase {

};
struct ASTNode : public ASTBase {

};
typedef ASTNode *ASTNodePtr;
struct ASTInfixNode : public ASTNode {
    ASTNodePtr m_left;
    std::string m_infix;
    ASTNodePtr m_right;
    ASTInfixNode(ASTNodePtr left, std::string infix, ASTNodePtr right) : m_left(std::move(left)), m_infix(std::move(infix)),
                                                                                m_right(std::move(right)) {}
};
struct ASTIdentifierNode : public ASTNode {
    std::string m_identifier;
    explicit ASTIdentifierNode(std::string identifier) : m_identifier(std::move(identifier)) {}
};

void mylalrtest() {
    const char *grammer = R"(
    Cpp {
        %left '+' '-';
        %left '*' '/';
        %whitespace "[ \t\r\n]*";
        expr:
            expr '+' expr [infix] | expr '-' expr [infix] | expr '*' expr [infix] | expr '/' expr [infix] |
            '(' expr ')' [compound] | identifier [identifier] | number [identifier] ;
        number: "[0-9]+";
        identifier: "[a-zA-Z_]+[0-9a-zA-Z_]*";
        chinese
    }
)";
    GrammarCompiler compiler;
    compiler.compile(grammer, grammer + strlen(grammer));
    Parser<const char *, ASTNodePtr> parser(compiler.parser_state_machine());

    parser.parser_action_handlers()
            ( "infix", [] ( const ASTNodePtr* data, const ParserNode<>* nodes, size_t length )
              {
                  return new ASTInfixNode(data[0], nodes[1].lexeme(), data[2]);
              })
              ( "compound", [] ( const ASTNodePtr* data, const ParserNode<>* nodes, size_t length )
              {
                  return data[1];
              })( "identifier", [] ( const ASTNodePtr* data, const ParserNode<>* nodes, size_t length )
              {
                  printf("%s ", nodes[0].lexeme().c_str());
                  return new ASTIdentifierNode(nodes[0].lexeme());
              });
    const char* input = "1 + 2 * 2 + _abc1234";
    parser.parse(input, input + strlen(input));


}

int main (int argc, char * const argv[]) {
    Document doc(nullptr);
    //doc.appendElement(new SubElement());
    doc.append(new SubElement());
    doc.append(new SubElement());
    doc.append(new SubElement());

    printf("line number -> %d \n", doc.getLineNumber());
    EventContext root(&doc);
    printf("is dcoument -> %d \n", root.isDocument());
    //root.enter(-3).nearby(-1).dump();
    EventContext sub = root.enter(0);
    sub.dump();
    sub.prev();

    sub.dump();

    return 0;
}

