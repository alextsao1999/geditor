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
    None,
    Change,
    Add,
    Delete,
    AddLine,
    DeleteLine
};

union CommandData {
    CommandData() : value(0) {}
    CommandData(int pos, int ch) : input(InputData(pos, ch)) {}
    struct InputData {
        int pos;
        int ch;
        InputData(int pos, int ch) : pos(pos), ch(ch) {}
    } input;
    int value;

};

struct Command {
    EventContext *context;
    CaretPos pos;
    CommandType type;
    CommandData data;
};

class CommandQueue {
    std::deque<Command> queue;
public:
    void push(Command cmd) {
        queue.push_back(cmd);
    };
    bool has() {
        return !queue.empty();
    }
    Command pop() {
        Command cmd = queue.back();
        queue.pop_back();
        return cmd;
    };

};


#endif //TEST_COMMAND_QUEUE_H
