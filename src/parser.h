//
// Created by Alex on 2020/2/26.
//

#ifndef GEDITOR_PARSER_H
#define GEDITOR_PARSER_H

#include <istream>
class EventContext;
class Parser {
public:
    typedef std::wistream istream;
    virtual void parse(EventContext *, istream &in) = 0;
};

class SimpleParser : public Parser {
public:
    void parse(EventContext *context, istream &in) override;

};

#endif //GEDITOR_PARSER_H
