#ifndef GPUMENU_H
#define GPUMENU_H

#include <QMenu>
#include <opencv2/core/cuda.hpp>

#include "mainwindow.h"

class MainWindow;

using namespace cv;

class GpuMenu : public QMenu
{
    Q_OBJECT

public:
    explicit GpuMenu(MainWindow *parent);
    ~GpuMenu();

public slots:
    void buildGpuList();
    void selectGpu(QAction * action);
    void selectGpu(int num);

private:
    MainWindow * mainWindow;

    QMenu * gpuMenuList;

    int nGpu;
};

#endif // GPUMENU_H
