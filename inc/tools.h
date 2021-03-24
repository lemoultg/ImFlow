#ifndef TOOLS_H
#define TOOLS_H

#include <QString>
#include <opencv2/core.hpp>
#include "mat.h"

using namespace cv;

void saveToMatlab_CV32(Mat flow,QString fileName="/media/glemoult/DATA/LOMC/Real Time PIV/RealTimePIV-QtApp/Flows/test.mat");
// Save flow directly to matlab

// QStringList functions
QString findCommonStart(const QStringList& list);
QString findCommonEnd(const QStringList& list);


#endif // TOOLS_H
