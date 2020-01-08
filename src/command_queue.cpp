//
// Created by Alex on 2019/6/26.
//

#include "command_queue.h"
#include "event.h"
#include "document.h"
void CommandQueue::handle(Command cmd) {
    //printf("handle command\n");
    if (cmd.type == CommandType::DeleteElement) {
        // free element
        printf("free element -> ");
        cmd.context->tag().dump();
        cmd.data.element->free();
    }
    if (cmd.type == CommandType::ReplaceElement) {
        printf("free repalce -> ");
        // free context && element
        cmd.context->tag().dump();
        cmd.context->element->free(); //被替换了 旧的元素释放
        cmd.data.replace.caret->free();
        //cmd.data.replace.element;
    }
    cmd.context->free();
}
