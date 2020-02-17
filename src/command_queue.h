//
// Created by Alex on 2019/6/26.
//

#ifndef _COMMAND_QUEUE_H
#define _COMMAND_QUEUE_H

#include <queue>
#include "common.h"
#include "caret_manager.h"

class Element;
struct EventContext;

enum class CommandType {
    PushStart,
    PushEnd,
    AddChar,
    SetString,
    DeleteChar,
    DeleteString,
    AddElement,
    DeleteElement,
    ReplaceElement,
    SeparateElement,
    Break,
    Combine,
};

union CommandData {
    explicit CommandData(int value = 0) : value(value) {}
    CommandData(int pos, int ch) : input(InputData(pos, ch)) {}
    CommandData(EventContext *context, Element *element) : replace(ReplaceData(context, element)) {}
    explicit CommandData(Element *element) : element(element) {}
    explicit CommandData(int pos, std::wstring *string) : string(StringData(pos, string)) {}
    struct InputData {
        int pos;
        int ch;
        InputData(int pos, int ch) : pos(pos), ch(ch) {}
    } input;
    struct ReplaceData {
        EventContext *caret; // caret context
        Element *element;
        ReplaceData(EventContext *context, Element *element) : caret(context), element(element) {}
    } replace;
    struct StringData {
        int pos;
        std::wstring *string;
        StringData(int pos, std::wstring *string) : pos(pos), string(string) {}
    } string;
    Element *element; // new element
    int value;
};

struct Command {
    EventContext *context; // push context
    CaretPos pos; // save caret pos
    CommandType type; // command type
    CommandData data; // push data
};

class CommandQueue {
    std::deque<Command> queue;
    const int maxStack = 1024;
    int count = 0;
public:
    void push(Command cmd) {
        if (count > maxStack) {
            handle(queue.front());
            queue.pop_front();
        } else {
            count++;
        }
        queue.push_back(cmd);
    };
    bool has() { return !queue.empty(); }
    Command pop() {
        count--;
        Command cmd = queue.back();
        queue.pop_back();
        return cmd;
    };
    static void handle(Command cmd);

};


#endif //TEST_COMMAND_QUEUE_H
