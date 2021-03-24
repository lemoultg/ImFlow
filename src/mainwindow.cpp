#include "gimage.h"
#include "gimagefield.h"
#include "gimagemulti.h"
#include "optflow.h"
#include "filebrowser.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "pivengine.h"
#include "gimport.h"
#include "processingoptions.h"
#include "imagedisplay.h"
#include "tools.h"
#include "gpumenu.h"

#include <QTimer>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QComboBox>
#include <QStringListModel>
#include <QToolBar>
#include <QFileDialog>
#include <QLineEdit>
#include <QLabel>
#include <QDragEnterEvent>
#include <QSplitter>
#include <QListView>
#include <QTreeView>
#include <QTextEdit>
#include <QGridLayout>
#include <QPushButton>
#include <QElapsedTimer>
#include <QtDebug>
#include <QTabWidget>

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/core.hpp"
#include <opencv2/core/cuda.hpp>

#include <iostream>

using namespace cv;
using namespace cv::cuda;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    imageModel = new ModelImagePair(this);

    //setAcceptDrops(true);




    // Build menu bar
    menu = this->menuBar();
    menu->addMenu("File");

    folderView = new QTreeView(this);
    folderView->setSelectionMode(QAbstractItemView::SingleSelection);
    importTab = new GImport(imageModel,this);
    processTab = new ProcessingOptions(this);
    batchTab = new BatchProcess(imageModel,this);

    //Add navigation folder bar
    fileBrowser=new FileBrowser(this,folderView);

    // Add all widgets to the main window.
    ui->tabMenu->addTab(folderView,"Directory");
    ui->tabMenu->addTab(importTab,"Image Pairs");
    ui->tabMenu->addTab(processTab,"Processing");
    ui->tabMenu->addTab(batchTab,"Batch");

    imMulti = new GImageMulti(imageModel,this);
    ui->splitter->addWidget(imMulti);

    //Builg GPU menu
    GpuMenu * gpuMenu = new GpuMenu(this);
    ui->menuBar->addMenu(gpuMenu);

    // Set-up connections
    connect(fileBrowser,SIGNAL(folderChanged(QDir)),importTab,SLOT(setDir(QDir)));

//    // Create optflow and piv engines
//    optFlow = new OptFlow();
//    piv = new PivEngine;


    //QElapsedTimer timer;
    //timer.start();
    //qDebug() << "optFlow took" << timer.elapsed() << "milliseconds";
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::displayStatus(QString message,int duration)
{
    ui->statusBar->showMessage(message,duration);
}

//void MainWindow::processPair()
//{
//    if (imA.empty() || imB.empty()) {
//        displayStatus("No image to process.");
//        return;
//    }

//    if (imA.size!=imB.size) {
//        displayStatus("image sizes don't match.");
//        return;
//    }

//    Mat tempFlow;

//    QList<TreeItem*> processList = getProcessList();

//    for (int i=0; i<processList.size(); i++) {
//        if (processList.at(i)->getType()==ProcessType::Piv) {
//            PivItem* process = static_cast<PivItem*>(processList.at(i));
//            PivOptions options = process->getOptions();

//            piv->loadImages(imA,imB);
//            piv->setOptions(options);
//            piv->generateGridPoint();
//            if (options.usePrevious)
//                piv->calcFlow(tempFlow);
//            else
//                piv->calcFlow();
//            piv->flowValidation();
//            piv->flow().copyTo(tempFlow);

//            imPIV->setField(tempFlow);
//            imPIV->displayMag();
//        }
//        else if (processList.at(i)->getType()==ProcessType::OpticalFlow) {
//            OptFlowItem* process = static_cast<OptFlowItem*>(processList.at(i));
//            OptFlowOptions options = process->getOptions();

//            optFlow->loadImages(imA,imB);
//            optFlow->setOptions(options);
//            optFlow->initialize(Size(imA.cols,imA.rows));
//            optFlow->calcFlow();

//            imFlow->setField(optFlow->flow());
//            imFlow->displayMag();
//        }
//    }


//}

QList<ImPairItem*> MainWindow::getImPairList()
{
    return importTab->getImPairList();
}

QList<TreeItem*> MainWindow::getProcessList()
{
    return processTab->getProcessList();

}


