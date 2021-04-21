#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "component.h"
#include "viewpainter.h"
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
    QList<Component> comps;
    QTabWidget *widget = new QTabWidget;
    setCentralWidget(widget);
    QWidget *newWidget = new QWidget();
    widget->addTab(newWidget, "New Tab");

    QLabel *label = new QLabel;
    QPixmap canvas(1920, 1080);
    canvas.fill();
    label->setPixmap(canvas);
    widget->setStyleSheet("background -colo: red:");
    widget->setAcceptDrops(true);
    setMouseTracking(true);
    widget->setTabsClosable(true);
    widget->setTabPosition(QTabWidget::South);

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
    tx = e->pos().x();
    ty = e->pos().y();
    cx1 = e->pos().x();
    cy1 = e->pos().y();
    qDebug() << e->pos();
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{
    qDebug() << e->pos();
}

void MainWindow::paintEvent(QPaintEvent *e)
{
    drawRes();
    update();
}

void MainWindow::drawRes()
{
//    QPainter painter(this);
//    QPen paintpen, pointpen;
//    QPoint p1, p2;
//    paintpen.setColor(Qt::darkBlue);
//    paintpen.setWidth(2);
//    pointpen.setColor(Qt::red);
//    pointpen.setWidth(4);

//    painter.setPen(paintpen);

//    //Resistor painting
//    painter.drawLine(tx-5, ty+8, tx+5, ty-8);
//    painter.drawLine(tx+5, ty-8, tx+15, ty+8);
//    painter.drawLine(tx+15, ty+8, tx+25, ty-8);
//    painter.drawLine(tx+25, ty-8, tx+30, ty);
//    painter.drawLine(tx+30, ty, tx+40, ty);
//    painter.drawLine(tx-5, ty+8, tx-15, ty-8);
//    painter.drawLine(tx-15, ty-8, tx-25, ty+8);
//    painter.drawLine(tx-25, ty+8, tx-30, ty);
//    painter.drawLine(tx-30, ty, tx-40, ty);

//    p1.setX(tx+40); p1.setY(ty);
//    p2.setX(tx-40); p2.setY(ty);

//    painter.setPen(pointpen);
//    painter.drawPoint(p1);
//    painter.drawPoint(p2);
}

void MainWindow::drawGND()
{

}

void MainWindow::drawCap()
{

}

void MainWindow::drawInd()
{

}

void MainWindow::drawWire()
{

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
