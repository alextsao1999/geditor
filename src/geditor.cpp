//
// Created by Alex on 2019/6/28.
//

#include "geditor.h"
#include "ast.h"

Document &EditorRender::target() {
    return m_data->current();
}
