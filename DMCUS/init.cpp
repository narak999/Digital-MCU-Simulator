#include "mainwindow.h"
#include "component.h"
#include "viewpainter.h"
#include <QMessageBox>
#include <QtWidgets>
#include <QMenu>
#include <QShortcut>
#include <QToolBar>
#include <QDockWidget>
#include <QPushButton>
#include <QAbstractScrollArea>
#include <QScrollArea>
#include <QComboBox>
#include <QStringList>
#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QList>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QInputDialog>
#include <QFile>

#ifndef QT_NO_CONTEXTMENU
void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(undoAct);
    menu.addAction(redoAct);
    menu.addSeparator();
    menu.addAction(cutAct);
    menu.addAction(copyAct);
    menu.addAction(pasteAct);
    menu.addAction(deleteAct);
    menu.exec(event->globalPos());
}

#endif // QT_NO_CONTEXTMENU

void MainWindow::createActions()
{
    //File Menu
    newAct = new QAction(QIcon((":/resources/icons/newDoc.png")),tr("&New Document"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);

    newTextAct = new QAction(QIcon(":/resources/icons/newTextDoc.png"), "New Text Document", this);
    newTextAct->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_V);
    newTextAct->setStatusTip("Creates a new text document");
    connect(newTextAct, &QAction::triggered, this, &MainWindow::newText);

    openAct = new QAction(QIcon((":/resources/icons/fileOpen.png")), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::open);

    closeAct = new QAction(QIcon(":/resources/icons/close.png"), "Close Document", this);
    closeAct->setShortcut(Qt::CTRL+Qt::Key_5);
    closeAct->setStatusTip("Closes the current document");
    connect(closeAct, &QAction::triggered, this, &MainWindow::close);

    saveAct = new QAction(QIcon(":/resources/icons/save.png"), "Save", this);
    saveAct->setShortcut(QKeySequence::Save);
    saveAct->setStatusTip("Saves the current document");
    connect(saveAct, &QAction::triggered, this, &MainWindow::save);

    saveAllAct = new QAction(QIcon(":/resources/icons/saveAll.png"), "Save All", this);
    saveAllAct->setShortcut(Qt::CTRL+Qt::Key_Plus);
    saveAllAct->setStatusTip("Saves all open documents");
    connect(saveAllAct, &QAction::triggered, this, &MainWindow::saveAll);

    saveAsAct = new QAction(QIcon(":/resources/icons/saveAs.png"), "Save As", this);
    saveAsAct->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_S);
    saveAsAct->setStatusTip("Saves the current document to a specific location");
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveAs);

    printAct = new QAction(QIcon(":/resources/icons/print.png"), "Print", this);
    printAct->setShortcut(QKeySequence::Print);
    printAct->setStatusTip("Prints the current document");
    connect(printAct, &QAction::triggered, this, &MainWindow::print);

    //Edit Menu
    redoAct = new QAction(QIcon(":/resources/icons/redo.png"), "Redo", this);
    redoAct->setShortcut(QKeySequence::Redo);
    redoAct->setStatusTip("Redoes the last command");
    connect(redoAct, &QAction::triggered, this, &MainWindow::redo);

    undoAct = new QAction(QIcon(":/resources/icons/undo.png"), "Undo", this);
    undoAct->setShortcut(QKeySequence::Undo);
    undoAct->setStatusTip("Undoes the last command");
    connect(undoAct, &QAction::triggered, this, &MainWindow::undo);

    cutAct = new QAction(QIcon((":/resources/icons/cut.png")), tr("&Cut"), this);
    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct, &QAction::triggered, this, &MainWindow::cut);

    copyAct = new QAction(QIcon((":/resources/icons/copy.png")), tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, &QAction::triggered, this, &MainWindow::copy);

    pasteAct = new QAction(QIcon((":/resources/icons/paste.png")), tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(pasteAct, &QAction::triggered, this, &MainWindow::paste);

    deleteAct = new QAction(QIcon(":/resources/icons/delete.png"), "Delete", this);
    deleteAct->setShortcut(QKeySequence::Delete);
    deleteAct->setStatusTip("Deletes the selected compontent(s)");
    connect(deleteAct, &QAction::triggered, this, &MainWindow::deleteComp);

    resAct = new QAction(QIcon(":/bitmaps/bitmaps/resistor.png"), "Build Resistors", this);
    resAct->setStatusTip("Enables Resistor Paintings");
    connect(resAct, SIGNAL(triggered()), this, SLOT(setResMode()));

    capAct = new QAction(QIcon(":/bitmaps/bitmaps/capacitor.png"), "Build Capacitors", this);
    capAct->setStatusTip("Enables Capacitor Paintings");
    connect(capAct, SIGNAL(triggered()), this, SLOT(setCapMode()));

    gndAct = new QAction(QIcon(":/bitmaps/bitmaps/gnd.png"), "Build Ground", this);
    gndAct->setStatusTip("Enables Ground Paintings");
    connect(gndAct, SIGNAL(triggered()), this, SLOT(setGNDMode()));

    indAct = new QAction(QIcon(":/bitmaps/bitmaps/inductor.png"), "Build Inductor", this);
    indAct->setStatusTip("Enables Inductor Paintings");
    connect(indAct, SIGNAL(triggered()), this, SLOT(setIndMode()));

    wireAct = new QAction(QIcon(":/resources/icons/battery.png"), "Build Wires", this);
    wireAct->setStatusTip("Creates a source");
    connect(wireAct, SIGNAL(triggered()), this, SLOT(setWireMode()));

    codeAct = new QAction(QIcon(":/bitmaps/bitmaps/code.png"), "Build Wires", this);
    codeAct->setStatusTip("Opens the IDE for microcontroller programming");
    connect(codeAct, SIGNAL(triggered()), this, SLOT(openIDE()));

    genNetlist = new QAction(QIcon(":/resources/icons/netlist.png"), "Generate Netlist", this);
    genNetlist->setStatusTip("Generates a SPICE Netlist");
    connect(genNetlist, SIGNAL(triggered()), this, SLOT(getNetlist()));

    //View Menu
    zoomResetAct = new QAction(QIcon(":/resources/icons/zoomMax.png"), "View 1:1", this);
    zoomResetAct->setShortcut(Qt::Key_0);
    zoomResetAct->setStatusTip("Resets view without magnification");
    connect(zoomResetAct, &QAction::triggered, this, &MainWindow::zoomReset);

    zoomFitAct = new QAction(QIcon(":/resources/icons/zoomFit.png"), "View all", this);
    zoomFitAct->setShortcut(Qt::Key_1);
    zoomFitAct->setStatusTip("Shows the whole page");
    connect(zoomFitAct, &QAction::triggered, this, &MainWindow::zoomFit);

    zoomInAct = new QAction(QIcon(":/resources/icons/zoomIn.png"), "Zoom in", this);
    zoomInAct->setShortcut(Qt::Key_Plus);
    zoomInAct->setStatusTip("Zooms in");
    connect(zoomInAct, &QAction::triggered, this, &MainWindow::zoomIn);

    zoomOutAct = new QAction(QIcon(":/resources/icons/zoomOut.png"), "Zoom out", this);
    zoomOutAct->setShortcut(Qt::Key_Minus);
    zoomOutAct->setStatusTip("Zooms out");
    connect(zoomOutAct, &QAction::triggered, this, &MainWindow::zoomOut);

    toggleToolbar = new QAction("Toggle Toolbar", this);
    toggleToolbar->setCheckable(true);
    toggleToolbar->setStatusTip("Enables/Disables the toolbar");
    connect(toggleToolbar, SIGNAL(toggled(bool)), SLOT(viewToolbar(bool)));

    aboutQtAct = new QAction("About Qt", this);
    aboutQtAct->setStatusTip("Shows information about the Qt toolkit");
    connect(aboutQtAct, SIGNAL(triggered()), this, SLOT(aboutQt()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(newTextAct);
    fileMenu->addAction(closeAct);
    fileMenu->addAction(openAct);
    fileMenu->addSeparator();
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAllAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addAction(printAct);

    editMenu = menuBar()->addMenu("Edit");
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addSeparator();
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);
    editMenu->addAction(deleteAct);

    viewMenu = menuBar()->addMenu("View");
    viewMenu->addAction(zoomResetAct);
    viewMenu->addAction(zoomFitAct);
    viewMenu->addAction(zoomInAct);
    viewMenu->addAction(zoomOutAct);
    viewMenu->addSeparator();
    viewMenu->addAction(toggleToolbar);

    aboutQtMenu = menuBar()->addMenu("About Qt...");
    aboutQtMenu->addAction(aboutQtAct);

}

void MainWindow::createToolbar()
{
    fileToolbar = new QToolBar(tr("File"));
    this->addToolBar(fileToolbar);
    fileToolbar->addAction(newAct);
    fileToolbar->addAction(newAct);
    fileToolbar->addAction(newTextAct);
    fileToolbar->addAction(closeAct);
    fileToolbar->addAction(openAct);
    fileToolbar->addAction(saveAct);
    fileToolbar->addAction(saveAllAct);
    fileToolbar->addAction(saveAsAct);
    fileToolbar->addAction(printAct);

    editToolbar = new QToolBar("Edit");
    this->addToolBar(editToolbar);
    editToolbar->addAction(undoAct);
    editToolbar->addAction(redoAct);
    editToolbar->addAction(cutAct);
    editToolbar->addAction(copyAct);
    editToolbar->addAction(pasteAct);
    editToolbar->addAction(deleteAct);
    editToolbar->addAction(resAct);
    editToolbar->addAction(capAct);
    editToolbar->addAction(gndAct);
    editToolbar->addAction(indAct);
    editToolbar->addAction(wireAct);
    editToolbar->addAction(codeAct);
    editToolbar->addAction(genNetlist);
}

void MainWindow::createDockWindow()
{
    QDockWidget *dock = new QDockWidget("Project Toolbox", this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea);
    dockTabs = new QTabWidget(dock);
    dockTabs->setTabPosition(QTabWidget::North);
    dock->setWidget(dockTabs);
    addDockWidget(Qt::LeftDockWidgetArea, dock);


    QWidget *projGroup = new QWidget();
    QVBoxLayout *ProjGroupLayout = new QVBoxLayout();
    QWidget *ProjButtons = new QWidget();
    QPushButton *ProjNew = new QPushButton(tr("New"));
    connect(ProjNew, SIGNAL(clicked()), SLOT(projButtonNew()));
    QPushButton *ProjOpen = new QPushButton(tr("Open"));
    connect(ProjOpen, SIGNAL(clicked()), SLOT(projButtonNew()));
    QPushButton *ProjDel = new QPushButton(tr("Delete"));
    connect(ProjDel, SIGNAL(clicked()), SLOT(projButtonNew()));

    QHBoxLayout *ProjButtonsLayout = new QHBoxLayout();
    ProjButtonsLayout->addWidget(ProjNew);
    ProjButtonsLayout->addWidget(ProjOpen);
    ProjButtonsLayout->addWidget(ProjDel);
    ProjButtons->setLayout(ProjButtonsLayout);

    ProjGroupLayout->addWidget(ProjButtons);

    QListView *projects = new QListView();
    ProjGroupLayout->addWidget(projects);
    projGroup->setLayout(ProjGroupLayout);
    dockTabs->addTab(projGroup, "Project");

}

void MainWindow::fillCompBox()
{
    //"Component Tab"
    QWidget *compGroup = new QWidget();

    QVBoxLayout *compGroupLayout = new QVBoxLayout();
    QHBoxLayout *compSearchLayout = new QHBoxLayout();

    QComboBox *compChoose = new QComboBox();
    QListWidget *compComp = new QListWidget();
    compComp->setViewMode(QListWidget::IconMode);
    compComp->setGridSize(QSize(110, 90));
    QLineEdit *compSearch = new QLineEdit();
    compSearch->setPlaceholderText("Type here to search component list");
    QPushButton *compSearchClear = new QPushButton("Clear");

    compGroupLayout->setSpacing(5);
    compGroupLayout->addWidget(compChoose);
    compGroupLayout->addWidget(compComp);
    compGroupLayout->addLayout(compSearchLayout);
    compSearchLayout->addWidget(compSearch);
    compSearchLayout->addWidget(compSearchClear);
    compGroup->setLayout(compGroupLayout);
    dockTabs->addTab(compGroup, "Components");

    //Hardcoding the component files for now, will clean this up later when adding full component list

    QString resistor_path = ":/bitmaps/bitmaps/resistor.png";
    QString cap_path = ":/bitmaps/bitmaps/capacitor.png";
    QString gnd_path = ":/bitmaps/bitmaps/gnd.png";
    QString DC_path = ":/bitmaps/bitmaps/dc_voltage.png";
    QString inductor_path = ":/bitmaps/bitmaps/inductor.png";

    QListWidgetItem *res = new QListWidgetItem(QIcon(resistor_path), "Resistor");
    compComp->addItem(res);
    QListWidgetItem *cap = new QListWidgetItem(QIcon(cap_path), "Capacitor");
    compComp->addItem(cap);
    QListWidgetItem *gnd = new QListWidgetItem(QIcon(gnd_path), "Ground");
    compComp->addItem(gnd);
    QListWidgetItem *dc = new QListWidgetItem(QIcon(DC_path), "DC_Source");
    compComp->addItem(dc);
    QListWidgetItem *induc = new QListWidgetItem(QIcon(inductor_path), "Inductor");
    compComp->addItem(induc);
}

void MainWindow::setResMode()
{
    Component newComponent;
    bool ok;
    newComponent.name = QInputDialog::getText(this, "Resistor Name", "Name:", QLineEdit::Normal, QDir::home().dirName(), &ok);
    newComponent.n1 = QInputDialog::getText(this, "Node 1", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
    newComponent.n2 = QInputDialog::getText(this, "Node 2", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
    newComponent.value = QInputDialog::getText(this, "Resistor Value", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
    newComponent.isSource = false;
    QString s = QString::number(resCounter + 1);
    newComponent.netName = "R" + s;
    resCounter++;
    comps.append(newComponent);
    for (int i = 0; i < comps.size(); i++){
        qDebug() << comps[i].value << " " << comps[i].name << Qt::endl;
    }
}

void MainWindow::setCapMode()
{
    Component newComponent;
    bool ok;
    newComponent.name = QInputDialog::getText(this, "Capacitor Name", "Name:", QLineEdit::Normal, QDir::home().dirName(), &ok);
    newComponent.n1 = QInputDialog::getText(this, "Node 1", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
    newComponent.n2 = QInputDialog::getText(this, "Node 2", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
    newComponent.value = QInputDialog::getText(this, "Capacitor Value", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
    newComponent.isSource = false;
    QString s = QString::number(capCounter + 1);
    newComponent.netName = "C" + s;
    capCounter++;
    comps.append(newComponent);
    for (int i = 0; i < comps.size(); i++){
        qDebug() << comps[i].value << " " << comps[i].name << Qt::endl;
    }
}

void MainWindow::setIndMode()
{
    Component newComponent;
    bool ok;
    newComponent.value = QInputDialog::getText(this, "Inductor Value", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
    newComponent.name = QInputDialog::getText(this, "Inductor Name", "Name:", QLineEdit::Normal, QDir::home().dirName(), &ok);
    newComponent.n1 = QInputDialog::getText(this, "Node 1", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
    newComponent.n2 = QInputDialog::getText(this, "Node 2", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
    newComponent.isSource = false;
    QString s = QString::number(indCounter + 1);
    newComponent.netName = "L" + s;
    indCounter++;
    comps.append(newComponent);
}

void MainWindow::setGNDMode()
{
    //QString v1, v2, td1, td2, tau1, tau2, pw, period, v_offset, v_peak, f, df, phase;
}

void MainWindow::setWireMode()
{
    Component newComponent;
    bool ok;
    newComponent.name = QInputDialog::getText(this, "Source Name", "Name:", QLineEdit::Normal, QDir::home().dirName(), &ok);
    newComponent.posNode = QInputDialog::getText(this, "Positive Node", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
    newComponent.negNode = QInputDialog::getText(this, "Negative Node", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
    newComponent.srcType = QInputDialog::getText(this, "Source type", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
    newComponent.isSource = true;
    QString s = QString::number(srcCounter + 1);
    newComponent.netName = "V" + s;

    if(newComponent.srcType == "DC"){
        newComponent.v1 = QInputDialog::getText(this, "Voltage", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
    }
    else if(newComponent.srcType == "EXP"){
        newComponent.v1 = QInputDialog::getText(this, "Initial Voltage", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
        newComponent.v2 = QInputDialog::getText(this, "Pulsed Value", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
        newComponent.td1 = QInputDialog::getText(this, "Rise Delay Time", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
        newComponent.tau1 = QInputDialog::getText(this, "Rise Time Constant", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
        newComponent.td2 = QInputDialog::getText(this, "Fall Delay Time", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
        newComponent.tau2 = QInputDialog::getText(this, "Fall Time Constant", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
    }
    else if(newComponent.srcType == "PULSE"){
        newComponent.v1 = QInputDialog::getText(this, "First Value", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
        newComponent.v2 = QInputDialog::getText(this, "Second Value", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
        newComponent.td1 = QInputDialog::getText(this, "Delay Time", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
        newComponent.tr = QInputDialog::getText(this, "Rise Time", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
        newComponent.td2 = QInputDialog::getText(this, "Fall Time", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
        newComponent.pw = QInputDialog::getText(this, "Pulse Width", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
        newComponent.period = QInputDialog::getText(this, "Period", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
    }
   else if(newComponent.srcType == "SIN"){
        newComponent.v_offset = QInputDialog::getText(this, "Voltage Offset", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
        newComponent.v_peak = QInputDialog::getText(this, "Amplitude", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
        newComponent.f = QInputDialog::getText(this, "Frequency", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
        newComponent.td1 = QInputDialog::getText(this, "Time Delay", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
        newComponent.df = QInputDialog::getText(this, "Damping Factor", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
        newComponent.phase = QInputDialog::getText(this, "Phase", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
    }
    srcCounter++;
    comps.append(newComponent);
}

void MainWindow::openIDE()
{
    //QProcess *process = new QProcess(this);
    QString file("\"/home/ak/Desktop/DMCUS/DMCUS/dist/DMCUS_IDE/DMCUS_IDE\"");
    QProcess::startDetached("\"/home/ak/Desktop/DMCUS/DMCUS/dist/DMCUS_IDE/DMCUS_IDE\"");
}

void MainWindow::setLayout()
{

}

void MainWindow::newFile()
{
    comps.clear();
}

void MainWindow::getNetlist()
{
    bool ok;
    QFile file("/home/ak/Desktop/DMCUS/DMCUS/netlist.txt");

    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, "File Error", "File was unable to be opened properly.");
    }

    QTextStream out(&file);
    QString netTitle = QInputDialog::getText(this, "Netlist title", "Value:", QLineEdit::Normal, QDir::home().dirName(), &ok);
    out << QString(".title %1").arg(netTitle) << Qt::endl;

    for (int i = 0; i < comps.size(); ++i){
        if(comps[i].isSource){
            if (comps[i].srcType == "DC"){
                QString s = QString("%1 %2 %3")
                            .arg(comps[i].posNode)
                            .arg(comps[i].negNode)
                            .arg(comps[i].v1);
                out << comps[i].name << " " << s << Qt::endl;
            }
            else if(comps[i].srcType == "EXP"){
                QString s = QString("%1 %2 %3(%5 %6 %7 %8)")
                            .arg(comps[i].posNode)
                            .arg(comps[i].negNode)
                            .arg(comps[i].srcType)
                            .arg(comps[i].v1)
                            .arg(comps[i].v2)
                            .arg(comps[i].td1)
                            .arg(comps[i].tau1);
                out << comps[i].name << " " << s << Qt::endl;
            }
            else if(comps[i].srcType == "PULSE"){
                QString s = QString("%1 %2 dc 0 %3(%4 %5 %6 %7 %8 %9 %10)")
                            .arg(comps[i].posNode)
                            .arg(comps[i].negNode)
                            .arg(comps[i].srcType)
                            .arg(comps[i].v1)
                            .arg(comps[i].v2)
                            .arg(comps[i].td1)
                            .arg(comps[i].tr)
                            .arg(comps[i].td2)
                            .arg(comps[i].pw)
                            .arg(comps[i].period);
                out << comps[i].name << " " << s << Qt::endl;
            }
            else if(comps[i].srcType == "SIN"){
                QString s = QString("%1 %2 %3(%4 %5 %6)")
                            .arg(comps[i].posNode)
                            .arg(comps[i].negNode)
                            .arg(comps[i].srcType)
                            .arg(comps[i].v_offset)
                            .arg(comps[i].v_peak)
                            .arg(comps[i].f)
                            .arg(comps[i].td1)
                            .arg(comps[i].df)
                            .arg(comps[i].phase);
                out << comps[i].name << " " << s << Qt::endl;
            }
        }
        else {
            out << comps[i].name << QString(" %1 %2 ").arg(comps[i].n1).arg(comps[i].n2) << comps[i].value << Qt::endl;
        }
    }
out << ".end";
}

void MainWindow::newText()
{

}

void MainWindow::open()
{

}

void MainWindow::save()
{

}

void MainWindow::saveAll()
{

}

void MainWindow::saveAs()
{

}

void MainWindow::closeDoc()
{

}

void MainWindow::cut()
{

}

void MainWindow::copy()
{

}

void MainWindow::paste()
{

}

void MainWindow::print()
{

}

void MainWindow::undo()
{

}

void MainWindow::redo()
{

}

void MainWindow::deleteComp()
{

}

void MainWindow::zoomReset()
{

}

void MainWindow::zoomFit()
{

}

void MainWindow::zoomOut()
{

}

void MainWindow::zoomIn()
{

}

void MainWindow::projButtonNew()
{

}

void MainWindow::aboutQt()
{
QMessageBox::aboutQt(this);
}
