#include "optflow.h"

#include <opencv2/cudaoptflow.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <QtDebug>

using namespace cv;
using namespace cv::cuda;

OptFlow::OptFlow() : QObject()
{
    // Optical flow using Farneback method
    d_OptFlowFarneBack = cv::cuda::FarnebackOpticalFlow::create();
}

void OptFlow::initialize(Size imSizeIn)
{
    imSize=imSizeIn;

    d_srcA.create(imSize,CV_8U);
    d_srcB.create(imSize,CV_8U);
    d_flow.create(imSize,CV_32FC2);

    d_OptFlowFarneBack->calc(d_srcA,d_srcB,d_flow);
}

bool OptFlow::loadImages(Mat imA, Mat imB)
{
    h_srcA=imA;
    h_srcB=imB;

    //Check that both images have the same size
    if ( (Size(h_srcA.cols,h_srcB.rows) != imSize) | (Size(h_srcB.cols,h_srcB.rows) != imSize) ) {
        return true;
        std::cout << "Sizes don't match" << std::endl;
    }

    return false;
}

void OptFlow::calcFlow()
{
    d_srcA.upload(h_srcA);
    d_srcB.upload(h_srcB);
    d_OptFlowFarneBack->calc(d_srcA,d_srcB,d_flow);
    d_flow.download(h_flow);
}

Mat OptFlow::flow() {return h_flow;}

void OptFlow::setOptions(OptFlowOptions options)
{
    d_OptFlowFarneBack.get()->setNumLevels(options.numLevels);
    d_OptFlowFarneBack.get()->setPyrScale(options.pyrScale);
    d_OptFlowFarneBack.get()->setFastPyramids(options.fastPyramids);
    d_OptFlowFarneBack.get()->setWinSize(options.winSize);
    d_OptFlowFarneBack.get()->setNumIters(options.numIters);
    d_OptFlowFarneBack.get()->setPolyN(options.polyN);
    d_OptFlowFarneBack.get()->setPolySigma(options.polySigma);
    d_OptFlowFarneBack.get()->setFlags(options.flags);
}
