#include "tools.h"

#include <QString>
#include <QStringList>
#include <opencv2/core.hpp>
#include "mat.h"

using namespace cv;

void saveToMatlab_CV32(Mat flow,QString fileName)
// Save flow directly to matlab
{
    mwSize rows=flow.rows;
    mwSize cols=flow.cols;
    mwSize nChannel=flow.channels();

    Mat channels[nChannel];
    split(flow,channels);

    //Allocate the array for matlab before copying data into it
    mwSize dims[] = {rows,cols,nChannel};
    mxArray *pVar = mxCreateNumericArray(3, dims, mxSINGLE_CLASS, mxREAL);
    mxSingle* ptr = mxGetSingles(pVar);

    //Copy data into matlab array
    for (mwSize i=0; i<nChannel; i++) {
        cv::transpose(channels[i], channels[i]); //remember to transpose first because MATLAB is col-major!!!
        memcpy(ptr+i*(rows*cols), channels[i].data,rows*cols*sizeof(CV_32F));
    }

    //Write into matlab file
    MATFile * matFile;
    QByteArray filePath = fileName.toLocal8Bit();
    matFile = matOpen(filePath.data(),"w");
    matPutVariable(matFile,"flow",pVar);

    //Clean
    mxDestroyArray(pVar);
    matClose(matFile);
}

QString findCommonStart(const QStringList& list)
{
    if (list.isEmpty())
        return QString();
    QString root = list.first();
    foreach (QString item,list) {
        while ( !item.startsWith(root)) {
            root=root.left(root.length()-1);
            if (root.isEmpty() )
                return QString();
        }
    }

    return root;
}

QString findCommonEnd(const QStringList& list)
{
    if (list.isEmpty())
        return QString();
    QString root = list.first();
    foreach (QString item,list) {
        while ( !item.endsWith(root)) {
            root=root.right(root.length()-1);
            if (root.isEmpty() )
                return QString();
        }
    }

    return root;
}
