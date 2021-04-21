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
    QString name, netName, value, n1, n2, acdc, srcType;
    QString v1, v2, td1, td2, tr, tau1, tau2, pw, period, v_offset, v_peak, f, df, phase,
            posNode, negNode;
    bool isSource;

    int  tx, ty;
};

#endif // COMPONENT_H
