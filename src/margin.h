//
// Created by Alex on 2020/2/26.
//

#ifndef GEDITOR_MARGIN_H
#define GEDITOR_MARGIN_H

#include <map>
#include "event.h"
class EventContext;
class Document;
class Offset;
class Margin {
public:
    Document *m_doc;
    int m_lineWidth = 0;
    int m_charWidth = 0;
    std::vector<int> m_widths = {10, 15};
    explicit Margin(Document *doc);
    void update();
    int width() {
        int result = m_lineWidth;
        for (int bar : m_widths) {
            result += bar;
        }
        return result;
    }
    int index(Offset offset);
    int offset(int index) {
        if (index == 1) {
            return 0;
        }
        int result = m_lineWidth;
        for (int i = 0; i < index - 2; i++) {
            result += m_widths[i];
        }
        return result;
    }
    void draw();
    void drawGutter(EventContext *context);


};


#endif //GEDITOR_MARGIN_H
