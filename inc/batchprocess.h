#ifndef BATCHPROCESS_H
#define BATCHPROCESS_H

#include "mainwindow.h"
#include "processingoptions.h"
#include "modelimagepair.h"

#include <QWidget>
#include <QDir>
#include <QStackedLayout>

class MainWindow;
class TreeItem;

namespace Ui {
class BatchProcess;
}

class BatchProcess : public QWidget
{
    Q_OBJECT

public:
    explicit BatchProcess(ModelImagePair * modelImagePairIn, MainWindow *parent = 0);
    ~BatchProcess();

public slots:
    void selectFolder();
    void batchProcessing();

private:
    MainWindow *mainWindow;
    Ui::BatchProcess *ui;

    ModelImagePair * modelImagePair;

    QStackedLayout *stackedLayout;

    QDir batchDirectory;
    QList<ImPairItem*> imPairList;
    QList<TreeItem *> processList;

};



#endif // BATCHPROCESS_H
