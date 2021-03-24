#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QDir>
#include <QFileSystemModel>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>

#include "filebrowser.h"
#include "optflow.h"
#include "gimage.h"
#include "gimagefield.h"
#include "gimagemulti.h"
#include "pivengine.h"
#include "gimport.h"
#include "processingoptions.h"
#include "batchprocess.h"
#include "modelimagepair.h"
#include "gpumenu.h"

class GpuMenu;
class OptFlow;
class FileBrowser;
class DropQLabel;
class ProcessingOptions;
class BatchProcess;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public slots:

    //void processPair();
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void displayStatus(QString message,int duration=1000);
    QTimer* Timer;

    QList<ImPairItem*> getImPairList();
    QList<TreeItem*> getProcessList();

private:
    Ui::MainWindow *ui;
    QMenuBar *menu;

    GpuMenu * gpuMenu;
    FileBrowser *fileBrowser;
    QTreeView *folderView;
    GImport *importTab;
    ProcessingOptions * processTab;
    BatchProcess * batchTab;

    ModelImagePair * imageModel;

    GImageMulti *imMulti;
};

#endif // MAINWINDOW_H
