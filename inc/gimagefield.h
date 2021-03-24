#ifndef GIMAGEFIELD_H
#define GIMAGEFIELD_H

#include <QObject>

#include "gimage.h"
#include "imagedisplay.h"

class GImageField : public GImage
{
    Q_OBJECT

public:
    explicit GImageField(QWidget *parent = 0);
    ~GImageField();
    void setField(Mat flowIn);
    void setImageScale(double scale);
    void computeVector(int step, double scale);
    void saveMatlab();

public slots:
    void displayMag();
    void displayUx();
    void displayUy();
    void displayVector(bool disp);
    void exportToMatlab();
private:
    QPushButton *buttonMag = new QPushButton("Mag",this);
    QPushButton *buttonUx = new QPushButton("Ux",this);
    QPushButton *buttonUy = new QPushButton("Uy",this);
    QPushButton *buttonVector = new QPushButton("Vec",this);
    QPushButton *buttonExport = new QPushButton("Export",this);

    Mat flow,flowx,flowy;
    QList<QGraphicsArrowItem*> arrowList;
};

#endif // GIMAGEFIELD_H
