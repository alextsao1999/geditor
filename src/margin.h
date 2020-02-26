//
// Created by Alex on 2020/2/26.
//

#ifndef GEDITOR_MARGIN_H
#define GEDITOR_MARGIN_H

class EventContext;
class Document;
class Offset;
class Margin {
public:
    Document *m_doc;
    int m_lineWidth = 0;
    int m_charWidth = 0;
    int m_folderWidth = 0;
    explicit Margin(Document *doc);
    void update();
    int width() {
        return m_lineWidth + 20;
    }
    int index(Offset offset);
    void draw();
    void drawGutter(EventContext *context);

};


#endif //GEDITOR_MARGIN_H
