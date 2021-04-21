#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "viewpainter.h"
#include "component.h"

QT_BEGIN_NAMESPACE
class QAction;
class QActionGroup;
class QLabel;
class QMenu;
class QToolBar;
class QTabWidget;
class QDockWidget;
class QListWidget;
class QListView;
class QAbstractScrollArea;
class QComboBox;
class QStringList;
class QString;
class QVBoxLayout;
class QHBoxLayout;
class QLineEdit;
class QMouseEvent;
class QProcess;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    //Tracking positions for paintings
    int tx, ty, cx1, cy1, cx2, cy2;

    int resCounter = 0;
    int capCounter = 0;
    int gndCounter = 0;
    int indCounter = 0;
    int wireCounter = 0;
    bool drawmode = false;
    int compChoice;

protected:
#ifndef QT_NO_CONTEXTMENU
    void contextMenuEvent(QContextMenuEvent *event) override;
#endif // QT_NO_CONTEXTMENU

    virtual void paintEvent(ViewPainter *e);
    virtual void mouseMoveEvent(QMouseEvent *e) override;
    virtual void mousePressEvent(QMouseEvent *e) override;

private slots:
    void newFile();
    void newText();
    void open();
    void save();
    void saveAll();
    void saveAs();
    void closeDoc();
    void print();
    void redo();
    void undo();
    void cut();
    void copy();
    void paste();
    void deleteComp();
    void zoomReset();
    void zoomFit();
    void zoomOut();
    void zoomIn();
    void aboutQt();
    void viewToolbar(bool toggle);
    void createDockWindow();
    void projButtonNew();
    void setResMode();
    void setCapMode();
    void setIndMode();
    void setGNDMode();
    void setWireMode();
    void openIDE();
    void getNetlist();

private:
    void createActions();
    void createMenus();
    void createToolbar();
    void fillCompBox();
    void setLayout();
    void drawRes(ViewPainter *e);
    void drawGND();
    void drawCap();
    void drawInd();
    void drawWire();

    QMenu *fileMenu, *editMenu, *viewMenu, *aboutQtMenu;
    QMenu *projMenu;
    QProcess *runExe;
    QAbstractScrollArea *scrollArea;
    QListView *projects;
    QListWidget *compComp;
    QDockWidget *dock;
    QTabWidget *dockTabs;
    QToolBar *fileToolbar, *editToolbar, *projToolbar;
    QAction *newAct, *openAct, *newTextAct, *closeAct, *saveAct, *saveAllAct, *saveAsAct,
            *redoAct, *undoAct, *deleteAct, *printAct, *cutAct, *pasteAct, *copyAct, *zoomResetAct,
            *zoomFitAct, *zoomInAct, *zoomOutAct, *toggleToolbar, *aboutQtAct, *resAct, *capAct, *indAct, *gndAct, *wireAct, *codeAct, *genNetlist;
    QComboBox *compChoose;
    QVBoxLayout *compGroup;
    QHBoxLayout *compSearchLayout;
    QLineEdit *compSearch;
    QList<Component> comps;
};
#endif // MAINWINDOW_H
