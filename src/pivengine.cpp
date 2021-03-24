#include "pivengine.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/photo.hpp>
#include <iostream>
#include <QtDebug>

#include "mat.h"

using namespace cv;

PivEngine::PivEngine()
{
    _xWinLen=options.xWinLen;
    _yWinLen=options.yWinLen;
    _xWinOver=options.xWinOver;
    _yWinOver=options.yWinOver;
}

bool PivEngine::loadImages(Mat imA, Mat imB)
{
    _imA=imA;
    _imB=imB;

    //Check that both images have the same size
    if (_imA.size != _imB.size) {
        return true;
        std::cout << "Sizes don't match" << std::endl;
    }

    _width = _imA.cols;
    _height = _imA.rows;

    return false;
}

void PivEngine::generateGridPoint()
{
    gridPointX.clear();
    for (int iX=_xWinLen/2; iX<_width-_xWinLen/2; iX+=_xWinLen-_xWinOver) {
        gridPointX.append(iX);
    }
    gridPointY.clear();
    for (int iY=_yWinLen/2; iY<_height-_yWinLen/2; iY+=_yWinLen-_yWinOver) {
        gridPointY.append(iY);
    }

    _flow.create(gridPointY.length(),gridPointX.length(),CV_32FC2);
    _peakRatio.create(gridPointY.length(),gridPointX.length(),CV_32F);

    //std::cout << "Number of grid points: " << gridPointX.length() << "x" << gridPointY.length() << std::endl;
}

void PivEngine::calcFlow()
{
    float xMaxSub,yMaxSub;


    // allocate temporary buffers and initialize them with 0's
    Mat subImA(2*_yWinLen,2*_xWinLen,_imA.type(), Scalar::all(0));

    // Get pointer for the center of the image
    Mat roiSubImA(subImA, Rect(_xWinLen/2,_yWinLen/2,_xWinLen,_yWinLen));

    Mat roiA,roiB, corrMap;

    for (int i=0; i<gridPointY.length(); i++) {

        Vec2f *row = _flow.ptr<Vec2f>(i);
        float *rowPeak = _peakRatio.ptr<float>(i);

        for (int j=0; j<gridPointX.length(); j++) {
            int x=gridPointX[j];
            int y=gridPointY[i];
            Range xRange(x-_xWinLen/2,x+_xWinLen/2);
            Range yRange(y-_yWinLen/2,y+_yWinLen/2);

            // Get pointer for roi in imB
            roiB=_imB(yRange,xRange);

            // Get pointer for roi in imA and copy it into the padded matrix
            roiA=_imA(yRange,xRange);
            subImA.setTo(Scalar(mean(roiA)));
            roiA.copyTo(roiSubImA);

            cv::matchTemplate(subImA,roiB,corrMap,TM_CCORR);

            //locate Max in corrMap
            double max;
            Point maxInd;
            minMaxLoc(corrMap,NULL,&max,NULL,&maxInd);

            //subPixel interpolation
            bool subPixInterp=true;
            if (subPixInterp) {
                subPixel(corrMap,maxInd.x,maxInd.y,xMaxSub,yMaxSub);
            }
            else {
                xMaxSub=maxInd.x;
                yMaxSub=maxInd.y;
            }

            //set pixel value
            row[j][0]=corrMap.cols/2-xMaxSub;
            row[j][1]=corrMap.rows/2-yMaxSub;
            rowPeak[j]=peakRatio(corrMap,max,maxInd);

        }

    }

    //resize(_flow,_flow,Size(0,0),_xWinLen-_xWinOver,_yWinLen-_yWinOver,INTER_NEAREST);
    //resize(_flow,_flow,Size(_imA.cols,_imA.rows),0,0,INTER_NEAREST);

}

void PivEngine::calcFlow(Mat flowIn)
{
    float xMaxSub,yMaxSub;

    //Resize flowIn to fit the new grid spacing
    resize(flowIn,flowIn,Size(_flow.cols,_flow.rows),0,0,INTER_CUBIC);

    // allocate temporary buffers and initialize them with 0's
    Mat subImA(2*_yWinLen,2*_xWinLen,_imA.type(), Scalar::all(0));

    // Get pointer for the center of the image
    Mat roiSubImA(subImA, Rect(_xWinLen/2,_yWinLen/2,_xWinLen,_yWinLen));

    Mat roiA,roiB, corrMap;

    for (int i=0; i<gridPointY.length(); i++) {

        Vec2f *row = _flow.ptr<Vec2f>(i);
        Vec2f *rowFlowIn = flowIn.ptr<Vec2f>(i);
        float *rowPeak = _peakRatio.ptr<float>(i);

        for (int j=0; j<gridPointX.length(); j++) {

            int x=gridPointX[j];
            int y=gridPointY[i];
            Range xRange(x-_xWinLen/2,x+_xWinLen/2);
            Range yRange(y-_yWinLen/2,y+_yWinLen/2);

            //get inFlow value at the interogation windows position
            int flowXtemp = (int)rowFlowIn[j][0];
            int flowYtemp = (int)rowFlowIn[j][1];

            // Get pointer for roi in imB using inFlow to shift the interogation window
            Range xRangeAdpt(x-_xWinLen/2+flowXtemp,x+_xWinLen/2+flowXtemp);
            Range yRangeAdpt(y-_yWinLen/2+flowYtemp,y+_yWinLen/2+flowYtemp);

            //Correct if the new interogation window is outside of the image
            if (xRangeAdpt.start<0)
                xRangeAdpt = Range(0,_xWinLen);
            else if (xRangeAdpt.end>_imA.cols)
                xRangeAdpt = Range(_imA.cols-_xWinLen,_imA.cols);

            if (yRangeAdpt.start<0)
                yRangeAdpt = Range(0,_yWinLen);
            else if (yRangeAdpt.end>_imA.rows)
                yRangeAdpt = Range(_imA.rows-_yWinLen,_imA.rows);

            roiB=_imB(yRangeAdpt,xRangeAdpt);


            // Get pointer for roi in imA and copy it into the padded matrix
            roiA=_imA(yRange,xRange);
            subImA.setTo(Scalar(mean(roiA)));
            roiA.copyTo(roiSubImA);

            cv::matchTemplate(subImA,roiB,corrMap,TM_CCORR);

            //locate Max in corrMap
            double max;
            Point maxInd;
            minMaxLoc(corrMap,NULL,&max,NULL,&maxInd);

            //subPixel interpolation
            bool subPixInterp=true;
            if (subPixInterp) {
                subPixel(corrMap,maxInd.x,maxInd.y,xMaxSub,yMaxSub);
            }
            else {
                xMaxSub=maxInd.x;
                yMaxSub=maxInd.y;
            }

            //set pixel value
            row[j][0]=corrMap.cols/2-xMaxSub+flowXtemp;
            row[j][1]=corrMap.rows/2-yMaxSub+flowYtemp;
            rowPeak[j]=peakRatio(corrMap,max,maxInd);
        }

    }

    //resize(_flow,_flow,Size(0,0),_xWinLen-_xWinOver,_yWinLen-_yWinOver,INTER_NEAREST);
    //resize(_flow,_flow,Size(_imA.cols,_imA.rows),0,0,INTER_NEAREST);

    if (0) {
        //plot
        Mat display;
        corrMap.copyTo(display);
        double min,max;
        minMaxLoc(display, &min, &max);
        if (min!=max)
            display.convertTo(display,CV_8U,255.0/(max-min),-255.0*min/(max-min));
        resize(display,display,Size(0,0),16,16,INTER_NEAREST);
        cv::namedWindow("test", WINDOW_AUTOSIZE );
        imshow("test",display);
        waitKey(0);
    }
}

void PivEngine::subPixel(Mat c, int xMax, int yMax, float &xMaxSub, float &yMaxSub)
// Subpixel interpolation adapted from OpenPIV_Matlab (sub_pixel_velocity_rect.m)
{
    float f0,f1,f2;

    f0 = log(c.at<float>(yMax,xMax));
    f1 = log(c.at<float>(yMax-1,xMax));
    f2 = log(c.at<float>(yMax+1,xMax));
    yMaxSub = float(yMax) + (f1-f2)/(2*f1-4*f0+2*f2);

    f0 = log(c.at<float>(yMax,xMax));
    f1 = log(c.at<float>(yMax,xMax-1));
    f2 = log(c.at<float>(yMax,xMax+1));
    xMaxSub = float(xMax) + (f1-f2)/(2*f1-4*f0+2*f2);

}

double PivEngine::peakRatio(Mat corrMap, double max, Point maxInd)
{
    Mat dilated, local_maxima;

    // Find all local maxima
    dilate(corrMap, dilated, Mat());
    compare(corrMap, dilated, local_maxima,CMP_EQ);

    // Remove global maxima
    local_maxima.at<uchar>(maxInd)=0;

    double seconMax;
    minMaxLoc(corrMap,NULL,&seconMax,NULL,NULL,local_maxima);

    Scalar meanCorrMap = mean(corrMap);

    if (seconMax>meanCorrMap[0])
        return (max-meanCorrMap[0])/(seconMax-meanCorrMap[0]);
    else
        return (max-meanCorrMap[0]);
}

void PivEngine::cleanFlow(float thresholdPeak) {
    Mat directions[2];
    Mat mask=_peakRatio<thresholdPeak;

    split(_flow,directions);

    cv::inpaint(directions[0],mask,directions[0],3,INPAINT_NS);
    cv::inpaint(directions[1],mask,directions[1],3,INPAINT_NS);
    merge(directions,2,_flow);
}

void PivEngine::flowValidation()
{
    Mat directions[2];
    Mat mask=_peakRatio<options.peakRatio;

    split(_flow,directions);

    cv::inpaint(directions[0],mask,directions[0],3,INPAINT_NS);
    cv::inpaint(directions[1],mask,directions[1],3,INPAINT_NS);
    merge(directions,2,_flow);
}

Mat PivEngine::flow() {return _flow;}

void PivEngine::setOptions(PivOptions optionsIn)
{
    options = optionsIn;

    _xWinLen=options.xWinLen;
    _yWinLen=options.yWinLen;
    _xWinOver=options.xWinOver;
    _yWinOver=options.yWinOver;
}

