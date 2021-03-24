#ifndef PIVENGINE_H
#define PIVENGINE_H

#include <opencv2/core.hpp>
#include <QList>

using namespace cv;

struct PivOptions
{
    int xWinLen=64;
    int xWinOver=0;
    int yWinLen=64;
    int yWinOver=0;
    bool validation=false;
    double peakRatio=1;
    double meanValue=0;
    bool usePrevious=false;
};

class PivEngine
{
public:
    PivEngine();
    bool loadImages(Mat imA, Mat imB);
    void generateGridPoint();
    void calcFlow();
    void calcFlow(Mat flowIn);
    void subPixel(Mat c, int xMax, int yMax, float &xMaxSub, float &yMaxSub);
    double peakRatio(Mat corrMap, double max, Point maxInd);
    void cleanFlow(float thresholdPeak);
    void flowValidation();

    //Acces functions
    Mat flow();
    void setOptions(PivOptions optionsIn);
private:
    Mat _imA,_imB;

    QList<int> gridPointX;
    QList<int> gridPointY;

    Mat _flow;
    Mat _peakRatio;

    int _xWinLen;
    int _xWinOver;
    int _yWinLen;
    int _yWinOver;
    int _width,_height;

    PivOptions options=PivOptions();
};

#endif // PIVENGINE_H
