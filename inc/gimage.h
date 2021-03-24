#ifndef GIMAGE_H
#define GIMAGE_H

#include "imagedisplay.h"

#include <QWidget>
#include <QLabel>
#include <QDir>
#include <QPushButton>
#include <QWidgetAction>

#include <QGraphicsScene>
#include <QGraphicsView>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#define COLORMAP_NUM_TOTAL 20

using namespace cv;

namespace Ui {
class GImage;
}

class GImage : public QWidget
{
    Q_OBJECT

public:
    explicit GImage(QWidget *parent = 0);
    ~GImage();
    Mat getMat();
public slots:
    void setImage(Mat imIn);
    void imRead(QDir filePath);
    void imRescale();

    void saveIm();
    void changeColorMap(QAction * action);

    void zoomDisplay(double zoom);
    void pixelInfoDisplay(int posX,int posY);

private:
    void updateImage();
    void drawColorMap();
    void buildColorMapMenu();
    void setColorMapRange(double min, double max);

    QDir imagePath;
    int imDepth;

    int colorMapNum=12;

protected:
    Ui::GImage *ui;

    Mat im, imColor, imScaled;

};



#endif // GIMAGE_H
