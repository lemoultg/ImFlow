#include "gimagefield.h"
#include "gimage.h"
#include "ui_gimage.h"

#include "tools.h"

#include <QtDebug>
#include <QFileDialog>

GImageField::GImageField(QWidget *parent) :
    GImage(parent)
{
    ui->buttonBar->insertWidget(6,buttonExport);
    ui->buttonBar->insertWidget(4,buttonVector);
    ui->buttonBar->insertSpacing(4,10);
    ui->buttonBar->insertWidget(4,buttonUy);
    ui->buttonBar->insertWidget(4,buttonUx);
    ui->buttonBar->insertWidget(4,buttonMag);
    ui->buttonBar->insertSpacing(4,10);

    buttonVector->setCheckable(true);

    connect(buttonMag,SIGNAL(clicked()),this,SLOT(displayMag()));
    connect(buttonUx,SIGNAL(clicked()),this,SLOT(displayUx()));
    connect(buttonUy,SIGNAL(clicked()),this,SLOT(displayUy()));
    connect(buttonVector,SIGNAL(clicked(bool)),this,SLOT(displayVector(bool)));
    connect(buttonExport,SIGNAL(clicked(bool)),this,SLOT(exportToMatlab()));

}

void GImageField::setField(Mat flowIn)
{
    Mat planes[2];
    split(flowIn, planes);

    flowIn.copyTo(flow);
    planes[0].copyTo(flowx);
    planes[1].copyTo(flowy);

    imColor.create(flow.rows,flow.cols,CV_8UC3);
    ui->imDisplay->setImage(imColor);
}

void GImageField::setImageScale(double scale)
{
    ui->imDisplay->setImageScale(scale);
}

void GImageField::displayMag()
{
    Mat mag;
    magnitude(flowx,flowy,mag);
    mag.copyTo(im);
    imRescale();
}

void GImageField::displayUx()
{
    flowx.copyTo(im);
    imRescale();
}

void GImageField::displayUy()
{
    flowy.copyTo(im);
    imRescale();
}

void GImageField::computeVector(int step, double scale)
{
    Mat mag,ang;
    cv::cartToPolar(flowx,flowy,mag,ang,true);

    ui->imDisplay->clearVector(arrowList);
    arrowList.clear();
    QPoint temp;
    for (int i=step/2; i<flow.rows; i+=step)
    {
        float *rowMag = mag.ptr<float>(i);
        float *rowAng = ang.ptr<float>(i);

        for (int j=step/2; j<flow.cols; j+=step)
        {
            temp.setX(j);
            temp.setY(i);
            QGraphicsArrowItem * arrow = ui->imDisplay->addVector(j,i,scale*rowMag[j],rowAng[j]);
            arrow->setVisible(false);
            arrowList.append(arrow);
        }
    }
}

void GImageField::displayVector(bool disp)
{
    QGraphicsArrowItem* arrow;
    foreach (arrow, arrowList) {
        arrow->setVisible(disp);
    }
}

void GImageField::exportToMatlab()
{
    QString savePath = QFileDialog::getSaveFileName(this,"Select file","/media/glemoult/DATA/LOMC/Real Time PIV/RealTimePIV-QtApp/Flows","Matlab files (*.mat)");
    if (!savePath.isEmpty()) {
        saveToMatlab_CV32(flow,savePath);
    }
}

GImageField::~GImageField()
{

}
