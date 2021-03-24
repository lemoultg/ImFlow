#include "gimage.h"
#include "ui_gimage.h"

#include "imagedisplay.h"

#include <iostream>
#include <QtDebug>

#include <QDragEnterEvent>
#include <QMimeData>
#include <QLabel>
#include <QFileDialog>
#include <QScrollBar>
#include <QWidgetAction>
#include <QSignalMapper>
#include <QErrorMessage>
#include <QHBoxLayout>
#include <QMenu>

#include <QGraphicsScene>
#include <QGraphicsView>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;

GImage::GImage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GImage)
{
    ui->setupUi(this);

    connect(ui->saveIm,SIGNAL(clicked()),this,SLOT(saveIm()));

    connect(ui->imDisplay,SIGNAL(zoomUpdated(double)),this,SLOT(zoomDisplay(double)));
    connect(ui->imDisplay,SIGNAL(mouseMoved(int,int)),this,SLOT(pixelInfoDisplay(int,int)));
    connect(ui->zoomIn,SIGNAL(clicked(bool)),ui->imDisplay,SLOT(zoomIn()));
    connect(ui->zoomOut,SIGNAL(clicked(bool)),ui->imDisplay,SLOT(zoomOut()));
    connect(ui->zoomOrig,SIGNAL(clicked(bool)),ui->imDisplay,SLOT(zoomOrig()));
    connect(ui->zoomFit,SIGNAL(clicked(bool)),ui->imDisplay,SLOT(zoomFit()));

    connect(ui->imDisplay,SIGNAL(dropEventReceived(QDir)),this,SLOT(imRead(QDir)));

    //Setup colorbar
    drawColorMap();
    buildColorMapMenu();
    connect(ui->colorBarMin,SIGNAL(valueChanged(double)),this,SLOT(imRescale()));
    connect(ui->colorBarMax,SIGNAL(valueChanged(double)),this,SLOT(imRescale()));
    connect(ui->colorBarAuto,SIGNAL(clicked(bool)),this,SLOT(imRescale()));
}

void GImage::drawColorMap()
{
    Mat colorBarBase(1,256,CV_8U);
    Mat colorBarDisp;
    for (int i=0; i<256; i++) { colorBarBase.at<uchar>(0,i)=i;}
    applyColorMap(colorBarBase,colorBarDisp,cv::ColormapTypes(colorMapNum));
    cvtColor(colorBarDisp,colorBarDisp,COLOR_BGR2RGB);
    QImage colorMapQImage(colorBarDisp.data, colorBarDisp.cols, colorBarDisp.rows, colorBarDisp.step,QImage::Format_RGB888);

    ui->colorBarMenu->setIcon(QPixmap::fromImage(colorMapQImage).scaled(256,18));
}

void GImage::buildColorMapMenu()
{
    ui->colorBarMenu->setStyleSheet("QToolButton::menu-indicator { image:none; }");

    Mat colorBarBase(1,256,CV_8U);
    Mat colorBarDisp;
    for (int i=0; i<256; i++) { colorBarBase.at<uchar>(0,i)=i;}

    for (int i=0; i<COLORMAP_NUM_TOTAL; i++) {
        applyColorMap(colorBarBase,colorBarDisp,cv::ColormapTypes(i));
        cvtColor(colorBarDisp,colorBarDisp,COLOR_BGR2RGB);
        QImage colorMapQImage(colorBarDisp.data, colorBarDisp.cols, colorBarDisp.rows, colorBarDisp.step,QImage::Format_RGB888);

        QWidgetAction * action = new QWidgetAction(this);
        QLabel * label = new QLabel;
        label->setPixmap(QPixmap::fromImage(colorMapQImage));
        label->setMinimumHeight(15);
        label->setScaledContents(true);
        label->setFrameShape(QFrame::Box);
        action->setDefaultWidget(label);
        action->setData(i);

        ui->colorBarMenu->addAction(action);
    }

    connect(ui->colorBarMenu,SIGNAL(triggered(QAction*)),this,SLOT(changeColorMap(QAction *)));
}

void GImage::changeColorMap(QAction * action)
{
    colorMapNum=action->data().toInt();
    drawColorMap();
    imRescale();
}

Mat GImage::getMat() { return im; }

void GImage::imRead(QDir filePath)
{
    if (haveImageReader(filePath.path().toStdString())) {
        imagePath=filePath;
        im=cv::imread(imagePath.path().toStdString(),IMREAD_GRAYSCALE);
        imColor.create(im.rows,im.cols,CV_8UC3);
        ui->imDisplay->setImage(imColor);
        ui->status->setText(filePath.path());

        ui->colorBarMin->blockSignals(true);
        ui->colorBarMax->blockSignals(true);
        ui->colorBarMin->setValue(0);
        ui->colorBarMax->setValue(255);
        ui->colorBarMin->blockSignals(false);
        ui->colorBarMax->blockSignals(false);
        imRescale();

        if (im.depth()==CV_8U) {
            //setColorBarMin(0);
            //setColorBarMax(255);
        }
        else if (im.depth()==CV_16U) {
            //setColorBarMin(0);
            //setColorBarMax(65535);
        }
        //imRescale();
    }
}

void GImage::setImage(Mat imIn)
{
    im=imIn;
    imColor.create(im.rows,im.cols,CV_8UC3);
    ui->imDisplay->setImage(imColor);

    imRescale();
}

void GImage::updateImage() { ui->imDisplay->updateImage(); }

void GImage::setColorMapRange(double min,double max)
{
    ui->colorBarMin->blockSignals(true);
    ui->colorBarMax->blockSignals(true);
    ui->colorBarMin->setValue(min);
    ui->colorBarMax->setValue(max);
    ui->colorBarMin->blockSignals(false);
    ui->colorBarMax->blockSignals(false);
}

void GImage::imRescale()
{
    if (im.empty())
        return;

    double colorBarMin, colorBarMax;

    if (ui->colorBarAuto->isChecked()) {
        minMaxLoc(im, &colorBarMin, &colorBarMax);
        setColorMapRange(colorBarMin,colorBarMax);
    }
    else {
        colorBarMin=ui->colorBarMin->value();
        colorBarMax=ui->colorBarMax->value();
    }

    if (colorBarMin!=colorBarMax)
        im.convertTo(imScaled,CV_8U,255.0/(colorBarMax-colorBarMin),-255.0*colorBarMin/(colorBarMax-colorBarMin));
    applyColorMap(imScaled,imColor,cv::ColormapTypes(colorMapNum));
    cvtColor(imColor,imColor,COLOR_BGR2RGB);
    updateImage();
}

void GImage::saveIm()
{
    QString savePath = QFileDialog::getSaveFileName(this,"Select file",QDir::root().path());

    if (!savePath.isEmpty()) {
        try {
            cv::imwrite(savePath.toStdString(),im);
        }
        catch (const std::exception& e) {
            QErrorMessage errorMessage;
            errorMessage.showMessage(e.what());

            errorMessage.setFixedSize(500,200);
            errorMessage.exec();
        }
    }
}

void GImage::zoomDisplay(double zoom)
{
    ui->zoomInfo->setText(QString("%1%").arg(zoom*100,0,'f',0));
}

void GImage::pixelInfoDisplay(int posX,int posY)
{
    if (im.empty())
            return;

    if (im.type()==CV_8U) {
        uchar pixelValue = im.at<uchar>(posY,posX);
        ui->pixelInfo->setText(QString("(%1,%2) %3").arg(posX).arg(posY).arg(pixelValue));
    }
    else if (im.type()==CV_32F) {
        float pixelValue = im.at<float>(posY,posX);
        ui->pixelInfo->setText(QString("(%1,%2) %3").arg(posY).arg(posX).arg(pixelValue,0,'f',2));
    }
}

GImage::~GImage()
{
    delete ui;
}


