#ifndef MODELIMAGEPAIR_H
#define MODELIMAGEPAIR_H

#include <QWidget>
#include <QAbstractTableModel>
#include <QTableView>
#include <QDir>

#include <opencv2/core.hpp>

using namespace cv;

struct ImPairItem
{
    QString imA="";
    QString imB="";
    QString fullImA="";
    QString fullImB="";
    Mat imAref;
    Mat imBref;
    Mat imFlowU;
    Mat imFlowV;
};

class ModelImagePair : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ModelImagePair(QObject *parent = nullptr);
    ~ModelImagePair();
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());

    int getSelectedItem();
    void setActivatedItem(int row);
    void clear();

    int getSize();
    ImPairItem *getItem(int row);
    QList<ImPairItem*> getList();

signals:
    void selectedItemChanged(int row);

private:
    QList<ImPairItem*> listImPair;
    int activatedRow=-1;
};

#endif // MODELIMAGEPAIR_H
