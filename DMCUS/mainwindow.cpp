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
    qDebug() << e->pos();
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{
    qDebug() << e->pos();
}

void MainWindow::paintEvent(ViewPainter *e)
{

}

void MainWindow::drawRes(ViewPainter *e)
{

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
