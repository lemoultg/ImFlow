#include "gpumenu.h"

GpuMenu::GpuMenu(MainWindow *parent) : QMenu(parent), mainWindow(parent)
{
    setTitle("GPU Config");

    gpuMenuList = new QMenu("Select GPU",this);
    QAction * refreshGpuList = new QAction("Refresh List",this);

    addMenu(gpuMenuList);
    addAction(refreshGpuList);

    connect(gpuMenuList,SIGNAL(triggered(QAction*)),this,SLOT(selectGpu(QAction*)));
    connect(refreshGpuList,SIGNAL(triggered(bool)),this,SLOT(buildGpuList()));

    buildGpuList();
}

GpuMenu::~GpuMenu()
{

}

void GpuMenu::buildGpuList()
{
    gpuMenuList->clear();

    //Query number of CUDA devices
    nGpu=getCudaEnabledDeviceCount();
    if (nGpu>0) {
        for (int i=0; i<nGpu; i++) {
            cuda::DeviceInfo gpuInfo = cuda::DeviceInfo(i);

            QAction * temp = new QAction(this);
            temp->setText(QString("GPU %1: %2").arg(i).arg(gpuInfo.name()));
            temp->setData(i);
            temp->setCheckable(true);

            gpuMenuList->addAction(temp);
        }
        selectGpu(0);
        if (nGpu==1)
            mainWindow->displayStatus(QString("%1 CUDA enabled device detected and selected").arg(nGpu));
        else
            mainWindow->displayStatus(QString("%1 CUDA enabled devices detected. GPU 0 selected.").arg(nGpu));
    }
    else {
        QAction * temp = new QAction(this);
        temp->setText("No GPU device detected");
        temp->setEnabled(false);

        gpuMenuList->addAction(temp);

        if (nGpu==0)
            mainWindow->displayStatus("No CUDA enabled device found");
        if (nGpu==-1)
            mainWindow->displayStatus("OpenCV compiled without CUDA support");
    }

}

void GpuMenu::selectGpu(QAction * action)
{
    selectGpu(action->data().toInt());
}

void GpuMenu::selectGpu(int num){

    cuda::setDevice(num);

    mainWindow->displayStatus(QString("GPU %1 selected").arg(num));

    foreach (QAction * action, gpuMenuList->actions()) {
        action->setChecked(action->data().toInt()==num);
    }
}
