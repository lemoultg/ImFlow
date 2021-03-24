#ifndef OPTFLOW_H
#define OPTFLOW_H

#include <QObject>

#include <opencv2/core.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaoptflow.hpp>
#include <opencv2/video/tracking.hpp>

using namespace cv;
using namespace cv::cuda;

struct OptFlowOptions
{
    int numLevels=4;
    double pyrScale=0.5;
    bool fastPyramids=true;
    int winSize=16;
    int numIters=5;
    int polyN=5;
    double polySigma=1.1;
    int flags=OPTFLOW_USE_INITIAL_FLOW;
};

class OptFlow : public QObject
{
    Q_OBJECT
public:
    explicit OptFlow();
    void initialize(Size imSizeIn);
    bool loadImages(Mat imA, Mat imB);
    void calcFlow();

    //Acces functions
    Mat flow();
    void setOptions(OptFlowOptions options);
signals:

public slots:

private:
    Mat h_srcA, h_srcB, h_flow;
    Size imSize;

    GpuMat d_srcA, d_srcB, d_flow;
    Ptr<cv::cuda::FarnebackOpticalFlow> d_OptFlowFarneBack;
};



#endif // OPTFLOW_H
