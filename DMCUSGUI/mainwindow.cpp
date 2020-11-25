#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QtWidgets>
#include <QMenu>
#include <QShortcut>
#include <QToolBar>
#include <QDockWidget>
#include <QPushButton>
#include <QComboBox>
#include <QStringList>
#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QList>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QLabel>
MainWindow::MainWindow(){
    QWidget *widget = new QWidget;
    QLabel *label = new QLabel;
    QPixmap canvas(1920, 1080);
    canvas.fill();
    label->setPixmap(canvas);
    setCentralWidget(widget);
    widget->setAcceptDrops(true);
    setMouseTracking(true);
//    widget->setTabsClosable(true);
//    widget->setTabPosition(QTabWidget::South);
//    QWidget *newWidget = new QWidget();
//    widget->addTab(newWidget, "New Tab");

    QWidget *topFiller = new QWidget;
    topFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QWidget *bottomFiller = new QWidget;
    bottomFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(5, 5, 5, 5);
    layout->addWidget(topFiller);
    layout->addWidget(bottomFiller);
    //layout->addWidget(label);
    widget->setLayout(layout);

    createDockWindow();
    createActions();
    createMenus();
    createToolbar();
    viewToolbar(true);

    fillCompBox();

    QString message = tr("A context menu is available by right-clicking");
    statusBar()->showMessage(message);

    setWindowTitle(tr("DMCUS"));
    setMinimumSize(100, 100);
    resize(1280, 720);
}

void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    if ((e->pos().x() > 285) && drawmode)
    {
        tx = e->pos().x();
        ty = e->pos().y();
        cx2 = e->pos().x();
        cy2 = e->pos().y();
    }
    else if(!drawmode)
    {
        tx = 0; ty = 0;
    }
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{
    if (drawmode)
    {
        tx = e->pos().x();
        ty = e->pos().y();
        cx1 = e->pos().x();
        cy1 = e->pos().y();
    }
    else
    {
        tx = 0; ty = 0;
    }
}

void MainWindow::paintEvent(QPaintEvent *e)
{
    switch(compChoice)
    {
    case -500:
         drawRes();
    break;
    case -501:
         drawCap();
        break;
    case -502:
        drawGND();
        break;
    case -503:
        drawInd();
        break;
    case -504:
        drawWire();
        break;
    default:
        drawmode = false;
    }
    update();
}

void MainWindow::drawRes()
{
    QPainter painter(this);
    QPen paintpen, pointpen;
    QPoint p1, p2;
    paintpen.setColor(Qt::darkBlue);
    paintpen.setWidth(2);
    pointpen.setColor(Qt::red);
    pointpen.setWidth(4);

    painter.setPen(paintpen);

    //Resistor painting
    painter.drawLine(tx-5, ty+8, tx+5, ty-8);
    painter.drawLine(tx+5, ty-8, tx+15, ty+8);
    painter.drawLine(tx+15, ty+8, tx+25, ty-8);
    painter.drawLine(tx+25, ty-8, tx+30, ty);
    painter.drawLine(tx+30, ty, tx+40, ty);
    painter.drawLine(tx-5, ty+8, tx-15, ty-8);
    painter.drawLine(tx-15, ty-8, tx-25, ty+8);
    painter.drawLine(tx-25, ty+8, tx-30, ty);
    painter.drawLine(tx-30, ty, tx-40, ty);

    p1.setX(tx+40); p1.setY(ty);
    p2.setX(tx-40); p2.setY(ty);

    painter.setPen(pointpen);
    painter.drawPoint(p1);
    painter.drawPoint(p2);
}

void MainWindow::drawGND()
{
    QPainter painter(this);
    QPen paintpen, pointpen;
    QPoint p1;
    paintpen.setColor(Qt::darkBlue);
    paintpen.setWidth(2);
    pointpen.setColor(Qt::red);
    pointpen.setWidth(4);

    painter.setPen(paintpen);

    //GND painting
    painter.drawLine(tx, ty, tx, ty-10);
    painter.drawLine(tx-10, ty, tx+10, ty);
    painter.drawLine(tx-6, ty+5, tx+6, ty+5);
    painter.drawLine(tx-2, ty+10, tx+2, ty+10);
    p1.setX(tx); p1.setY(ty-10);
    painter.setPen(pointpen);
    painter.drawPoint(p1);
}

void MainWindow::drawCap()
{
    QPainter painter(this);
    QPen paintpen, pointpen;
    QPoint p1, p2;
    paintpen.setColor(Qt::darkBlue);
    paintpen.setWidth(2);
    pointpen.setColor(Qt::red);
    pointpen.setWidth(4);

    painter.setPen(paintpen);

    //Capacitor painting
    painter.drawLine(tx-3, ty+10, tx-3, ty-10);
    painter.drawLine(tx+3, ty+10, tx+3, ty-10);
    painter.drawLine(tx-3, ty, tx-16, ty);
    painter.drawLine(tx+3, ty, tx+16, ty);

    p1.setX(tx-16); p1.setY(ty);
    p2.setX(tx+16); p2.setY(ty);

    painter.setPen(pointpen);
    painter.drawPoint(p1);
    painter.drawPoint(p2);
}

void MainWindow::drawInd()
{
    QPainter painter(this);
    QPen paintpen, pointpen;
    QPoint p1, p2;
    paintpen.setColor(Qt::darkBlue);
    paintpen.setWidth(2);
    pointpen.setColor(Qt::red);
    pointpen.setWidth(4);

    painter.setPen(paintpen);

    //Inductor painting
    QRectF rectangle(tx, ty, 20, 30);
    QRectF rectangle1(tx+17, ty, 20, 30);
    QRectF rectangle2(tx+35, ty, 20, 30);
    int startAngle = 30 * 16;
    int spanAngle = 120 * 16;
    painter.drawArc(rectangle, startAngle, spanAngle);
    painter.drawArc(rectangle1, startAngle, spanAngle);
    painter.drawArc(rectangle2, startAngle, spanAngle);
    painter.drawLine(tx+55, ty+9, tx+65, ty+9);
    painter.drawLine(tx, ty+9, tx-10, ty+9);

    p1.setX(tx+65); p1.setY(ty+9);
    p2.setX(tx-10); p2.setY(ty+9);
    painter.setPen(pointpen);
    painter.drawPoint(p1);
    painter.drawPoint(p2);
}

void MainWindow::drawWire()
{
    QPainter painter(this);
    QPen paintpen, pointpen;
    paintpen.setColor(Qt::darkBlue);
    paintpen.setWidth(2);
    pointpen.setColor(Qt::red);
    pointpen.setWidth(4);

    //Wire painting
    painter.setPen(paintpen);
    painter.drawLine(cx1, cy1, cx2, cy1);
    painter.drawLine(cx2, cy1, cx2, cy2);
}

//Toggles toolbar on/off
void MainWindow::viewToolbar(bool toggle)
{
    fileToolbar->setVisible(toggle);
    editToolbar->setVisible(toggle);
}

MainWindow::~MainWindow()
{

}
