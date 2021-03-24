#include "batchprocess.h"
#include "ui_batchprocess.h"

#include "mainwindow.h"
#include "tools.h"

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

//Matlab support
#include "mat.h"

#include <QFileDialog>
#include <QDebug>
#include <QElapsedTimer>
#include <QStackedLayout>
#include <QFuture>
#include <QProgressDialog>

using namespace cv;

typedef cv::Vec<float, 2> Vec2float;

BatchProcess::BatchProcess(ModelImagePair * modelImagePairIn, MainWindow *parent) :
    QWidget(parent),
    mainWindow(parent),
    ui(new Ui::BatchProcess)
{
    ui->setupUi(this);
    modelImagePair=modelImagePairIn;

    connect(ui->buttonSelectFolder,SIGNAL(clicked(bool)),this,SLOT(selectFolder()));
    connect(ui->buttonStart,SIGNAL(clicked(bool)),this,SLOT(batchProcessing()));
}

BatchProcess::~BatchProcess()
{
    delete ui;
}

void BatchProcess::selectFolder()
{
    QString newFolder;
    newFolder = QFileDialog::getExistingDirectory(0,"Select folder for saving results");

    if (newFolder.isEmpty())
        return;

    batchDirectory = QDir(newFolder);
    ui->displayFolder->setText(batchDirectory.path());
}

//void BatchProcess::processImagePair()
//{
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

void BatchProcess::batchProcessing()
{
    bool saveMat=ui->checkBoxMat->isChecked();
    bool saveTiff=ui->checkBoxTiff->isChecked();

    if ( ( !batchDirectory.exists() || batchDirectory.path()=="." ) && ( saveMat || saveTiff ) ) {
        mainWindow->displayStatus("Selected folder does not exist.",4000);
        return;
    }

    imPairList=modelImagePair->getList();
    if (imPairList.isEmpty()) {
        mainWindow->displayStatus("No image pair to process.",4000);
        return;
    }

    processList=mainWindow->getProcessList();
//    if (processList.isEmpty()) {
//        mainWindow->displayStatus("No process to compute.",4000);
//        return;
//    }

    ui->progressBar->setRange(0,imPairList.size());
    ui->progressBar->setFormat("Computing ...");
    ui->progressBar->setValue(0);
    ui->stackedWidget->setCurrentIndex(1);

    OptFlow* engine = new OptFlow();
    OptFlowOptions options;
    options.winSize=32;
    //options.fastPyramids=false;
    options.polySigma=0.9;
    //options.pyrScale=0.8;
    //options.numLevels=10;
    engine->setOptions(options);

    //initialize optflow engine
    ImPairItem* temp = imPairList.first();
    engine->initialize(Size(temp->imAref.cols,temp->imAref.rows));

    Mat channels[2];
    Mat u,v;

    // Initialize matlab file
    mwSize rows=temp->imAref.rows;
    mwSize cols=temp->imAref.cols;
    mwSize nIm=imPairList.size();

    mwSize dims[] = {rows,cols,nIm};
    mxArray *pVarU = mxCreateNumericArray(3, dims, mxSINGLE_CLASS, mxREAL);
    mxSingle* ptrU = mxGetSingles(pVarU);
    mxArray *pVarV = mxCreateNumericArray(3, dims, mxSINGLE_CLASS, mxREAL);
    mxSingle* ptrV = mxGetSingles(pVarV);

    QString fileNameTiffBase = ui->fileNameTiff->text().isEmpty() ? batchDirectory.filePath("unnamed") : batchDirectory.filePath(ui->fileNameTiff->text());
    QString fileNameTiff;

    QElapsedTimer timer;
    timer.start();

    for (int i=0; i<imPairList.size();i++) {
        ui->progressBar->setValue(i);

        QApplication::processEvents();
        if (ui->buttonStop->isChecked())
            break;

        ImPairItem* temp = imPairList.at(i);
        engine->loadImages(temp->imAref,temp->imBref);
        engine->calcFlow();
        split(engine->flow(),channels);

        channels[0].copyTo(temp->imFlowU);
        channels[1].copyTo(temp->imFlowV);

        //Save to matlab file
        if (saveMat) {
            cv::transpose(channels[0],u); //remember to transpose first because MATLAB is col-major!!!
            cv::transpose(channels[1],v); //remember to transpose first because MATLAB is col-major!!!
            memcpy(ptrU+i*(rows*cols),u.data,rows*cols*sizeof(CV_32F));
            memcpy(ptrV+i*(rows*cols),v.data,rows*cols*sizeof(CV_32F));
        }

        //Save tiff file
        if (saveTiff) {
            fileNameTiff = QString("%1_u_%2.tiff").arg(fileNameTiffBase).arg(i);
            imwrite(fileNameTiff.toStdString(),channels[0]);
            fileNameTiff = QString("%1_v_%2.tiff").arg(fileNameTiffBase).arg(i);
            imwrite(fileNameTiff.toStdString(),channels[1]);
        }
    }

    double processTime = double(timer.elapsed())/1000.;

    if (!ui->buttonStop->isChecked()) {
        ui->progressBar->setFormat("Saving ...");
        ui->progressBar->setValue(imPairList.size());

        //Write into matlab file
        if (saveMat) {
            QString fileNameMat = ui->fileNameMat->text().isEmpty() ? "unnamed.mat" : QString("%1.mat").arg(ui->fileNameMat->text());
            fileNameMat = batchDirectory.filePath(fileNameMat);

            MATFile * matFile;
            QByteArray filePath = fileNameMat.toLocal8Bit();
            matFile = matOpen(filePath.data(),"wL");
            matPutVariable(matFile,"u",pVarU);
            matPutVariable(matFile,"v",pVarV);
            matClose(matFile);
        }

        double savingTime = double(timer.elapsed())/1000. - processTime;

        mainWindow->displayStatus(QString("Processing took %1 s - Saving took %2 s").arg(processTime,0,'f',1).arg(savingTime,0,'f',1),5000);
    }
    else {
        ui->buttonStop->setChecked(false);
        mainWindow->displayStatus(QString("The processing was aborted after %1 s").arg(processTime,0,'f',1),5000);
    }


    //Clean Matlab pointer
    mxDestroyArray(pVarU);
    mxDestroyArray(pVarV);

    ui->stackedWidget->setCurrentIndex(0);


}














