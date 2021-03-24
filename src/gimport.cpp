#include "gimport.h"
#include "ui_gimport.h"
#include "tools.h"

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <QStringListModel>
#include <QTableWidgetItem>
#include <QDir>
#include <QDebug>
#include <QList>
#include <QStringList>

using namespace cv;

GImport::GImport(ModelImagePair *modelImagePairIn, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GImport)
{
    ui->setupUi(this);
    modelImagePair=modelImagePairIn;
    ui->listAB->setModel(modelImagePair);

    QHeaderView* hHeader = ui->listAB->horizontalHeader();
    hHeader->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    hHeader->setMinimumSectionSize(10);
    hHeader->setSectionResizeMode(1,QHeaderView::Stretch);
    hHeader->setSectionResizeMode(2,QHeaderView::Stretch);
    QHeaderView* vHeader = ui->listAB->verticalHeader();
    vHeader->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->listAB->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(ui->autoImportButton,SIGNAL(clicked(bool)),this,SLOT(autoImport()));
    connect(ui->listAB,SIGNAL(activated(QModelIndex)),this,SLOT(pairActivated(QModelIndex)));
}

GImport::~GImport()
{
    delete ui;
}

void GImport::setDir(QDir dir) { workingDir=dir; }

QVector<QPair<QString, Mat> > GImport::loadAllImages()
{
    QVector<QPair<QString,Mat>> imList;

    workingDir.setFilter(QDir::Files | QDir::Readable);
    //QStringList filters;
    //filters << "*.bmp" << "*.jpeg" << "*.jpg" << "*.png" << "*.pgm" << "*.tiff" << "*.tif" ;
    //workingDir.setNameFilters(filters);
    workingDir.setSorting(QDir::Name | QDir::IgnoreCase);
    QStringList fileList = workingDir.entryList();

    foreach (QString item, fileList) {
        if (haveImageReader(workingDir.filePath(item).toStdString())) {
            Mat im=cv::imread(workingDir.filePath(item).toStdString(),IMREAD_GRAYSCALE);
            imList.append(QPair<QString,Mat>(item,im));
        }
    }

    return imList;
}

void GImport::autoImport()
{
    QVector<QPair<QString,Mat>> imList = loadAllImages();

    modelImagePair->clear();

    QStringList nameList;
    for (int i=0; i<imList.size(); i++) {
        QString item = imList.at(i).first;
        nameList.append(item);
    }

    QString start = findCommonStart(nameList);
    QString end = findCommonEnd(nameList);

    QStringList newList;
    //isolate variable part of the string
    for (int i=0; i<imList.size(); i++) {
        QString item = imList.at(i).first;
        item.remove(start);
        item.remove(end);
        newList.append(item);
    }

    QStringList listA, listB;
    // Check if files are sorted with "a" or "b" indices, if yes return list A and B
    listA = newList.filter("a",Qt::CaseInsensitive);
    listB = newList.filter("b",Qt::CaseInsensitive);
    if ( listA.size()==listB.size() && !listA.isEmpty() ) {
        for (int i=0; i<listA.size(); i++) {
            int indA = newList.indexOf(listA[i]);
            int indB = newList.indexOf(listB[i]);
            addPair(imList.at(indA),imList.at(indB));
        }
    }
    // If neither a nor b is found, fill a and b from the same list alternatively
    else {
        for (int i=0; i<imList.size()-1; i++) {
            addPair(imList.at(i),imList.at(i+1));
        }
    }

    if (modelImagePair->getSize()>0)
        modelImagePair->setActivatedItem(0);
}

QString GImport::findCommonStart(QStringList imList)
{
    if (imList.isEmpty())
        return QString();

    QString root = imList.first();
    foreach (QString item, imList) {
        while ( !item.startsWith(root)) {
            root=root.left(root.length()-1);
            if (root.isEmpty() )
                return QString();
        }
    }

    return root;
}

QString GImport::findCommonEnd(QStringList imList)
{
    if (imList.isEmpty())
        return QString();

    QString root = imList.first();
    foreach (QString item, imList) {
        while ( !item.endsWith(root)) {
            root=root.right(root.length()-1);
            if (root.isEmpty() )
                return QString();
        }
    }

    return root;
}

void GImport::addPair(QPair<QString,Mat> imA, QPair<QString,Mat> imB)
{
    int rows = modelImagePair->rowCount();

    modelImagePair->insertRow(rows);
    ImPairItem * temp = modelImagePair->getItem(rows);
    temp->imA=imA.first;
    temp->imB=imB.first;
    temp->fullImA=workingDir.filePath(imA.first);
    temp->fullImB=workingDir.filePath(imB.first);
    temp->imAref=imA.second;
    temp->imBref=imB.second;
    temp->imFlowU=Mat();
    temp->imFlowV=Mat();
}

void GImport::pairActivated(QModelIndex index)
{
    int row=index.row();
    modelImagePair->setActivatedItem(row);

    emit newPair(modelImagePair->getItem(row));
}

QList<ImPairItem*> GImport::getImPairList()
{
    return modelImagePair->getList();
}
