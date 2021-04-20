#include <cmath>
#include <stdlib.h>

#include "component.h"
#include "node.h"
#include "viewpainter.h"
#include <QPen>
#include <QString>
#include <QMessageBox>
#include <QPainter>
#include <QDebug>

Component::Component()
{
  Type = isAnalogComponent;

  cx = 0;
  cy = 0;
  tx = 0;
  ty = 0;
}

void Component::test()
{

}

Component* Component::newOne()
{
  return new Component();
}

void Component::paint(ViewPainter *p)
{
    Lines.append(new Line(-30, 0, -9, 0, QPen(Qt::darkBlue, 2)));
    foreach(Line *p1, Lines) {
      p->Painter->setPen(p1->style);
      p->drawLine(cx+p1->x1, cy+p1->y1, cx+p1->x2, cy+p1->y2);
    }
}
