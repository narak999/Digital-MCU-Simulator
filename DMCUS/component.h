#ifndef COMPONENT_H
#define COMPONENT_H

#include <QList>
#include "element.h"

class ViewPainter;
class QString;
class QPen;

class Component : public Element {
public:
    Component();
    virtual ~Component() {};

    void test();
    void paint(ViewPainter*);
    Component* newOne();
    QList<Line *>     Lines;
    QList<struct Arc *>      Arcs;
    QList<Area *>     Rects;
    QList<Area *>     Ellips;
    QList<Port *>     Ports;
    QList<Text *>     Texts;
    QString name;
    double value;


    int  tx, ty;
};

#endif // COMPONENT_H
