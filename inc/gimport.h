#ifndef GIMPORT_H
#define GIMPORT_H

#include "modelimagepair.h"

#include <QWidget>
#include <QAbstractTableModel>
#include <QTableView>
#include <QDir>

#include <opencv2/core.hpp>

using namespace cv;

class PairListModel;
struct ImPairItem;

namespace Ui {
class GImport;
}

class GImport : public QWidget
{
    Q_OBJECT

public:
    explicit GImport(ModelImagePair * modelImagePairIn, QWidget *parent = 0);
    ~GImport();

    void addPair(QPair<QString,Mat> imA, QPair<QString,Mat> imB);
    QList<ImPairItem*> getImPairList();

signals:
    void newPair(ImPairItem* imPair);
    void newImageA(QDir imA);
    void newImageB(QDir imB);

public slots:
    void autoImport();
    void setDir(QDir dir);
    void pairActivated(QModelIndex index);
    QVector<QPair<QString,Mat>> loadAllImages();

private:
    QString findCommonStart(QStringList imList);
    QString findCommonEnd(QStringList imList);

    Ui::GImport *ui;

    ModelImagePair * modelImagePair;

    QDir workingDir=QDir::current();
};

#endif // GIMPORT_H
