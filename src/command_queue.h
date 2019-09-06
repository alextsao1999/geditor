//
// Created by Alex on 2019/6/26.
//

#ifndef _COMMAND_QUEUE_H
#define _COMMAND_QUEUE_H

#include <queue>
#include "common.h"

struct Element;
struct EventContext;

enum class CommandType {
    None,
    Change,
    Add,
    Delete,
};

union CommandData {
    CommandData(int pos, int ch) : data(Data(pos, ch)) {

    }
    struct Data {
        int pos;
        int ch;
        Data(int pos, int ch) : pos(pos), ch(ch) {}
    } data;

};

struct Command {
    EventContext *context;
    CommandType type;
    CommandData data;
};

class CommandQueue {
    std::queue<Command> queue;
public:
    void push(Command cmd) {
        queue.push(cmd);
    };
    Command pop() {
        Command cmd = queue.back();
        queue.pop();
        return cmd;
    };

};


#endif //TEST_COMMAND_QUEUE_H
