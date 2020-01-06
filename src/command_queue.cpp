//
// Created by Alex on 2019/6/26.
//

#include "command_queue.h"
#include "event.h"
#include "document.h"
void CommandQueue::handle(Command cmd) {
    printf("handle command\n");
    if (cmd.type == CommandType::DeleteElement) {
        // free element
        cmd.data.element->free();
    }
    if (cmd.type == CommandType::ReplaceElement) {
        // free context && element
    }
    cmd.context->free();
}
