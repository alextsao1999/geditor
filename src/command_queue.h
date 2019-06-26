//
// Created by Alex on 2019/6/26.
//

#ifndef _COMMAND_QUEUE_H
#define _COMMAND_QUEUE_H

#include <queue>
struct Element;

enum class CommandType {
    None,
    Change,
    Add,
    Delete,
};
struct Command {
    Element *element;
    CommandType type;
    union {
        struct {
            int pos;
            const char *string;
        };
        struct {

        };
    } data;
};

class CommandQueue {
    std::queue<Command> queue;
public:
    void push(Command &&cmd) {
        queue.push(cmd);
    };
    Command pop() {
        Command cmd = queue.back();
        queue.pop();
        return cmd;
    };

};


#endif //TEST_COMMAND_QUEUE_H
