#include "mainwindow.h"
#include <QApplication>
#include <QtDebug>

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/core/cuda.hpp>

using namespace std;
using namespace cv;
using namespace cv::cuda;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    w.show();



    return a.exec();
}
