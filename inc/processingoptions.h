#ifndef PROCESSINGOPTIONS_H
#define PROCESSINGOPTIONS_H

#include <QWidget>
#include <QTreeView>
#include <QItemDelegate>

#include "mainwindow.h"
#include "optflow.h"
#include "pivengine.h"

class MainWindow;
class TreeModel;
class TreeItem;

namespace Ui {
class ProcessingOptions;
}

class ProcessingOptions : public QWidget
{
    Q_OBJECT

public:
    explicit ProcessingOptions(MainWindow *parent = 0);
    ~ProcessingOptions();
    QList<TreeItem *> getProcessList();

public slots:
    void addProcessing();
    void removeProcessing();
    void autoProcess();

signals:
    void autoProcessSignal();

private:
    Ui::ProcessingOptions *ui;

    MainWindow* mainWindow;
    TreeModel *model;
};


//---- Process Type -----//

enum ProcessType {
    Piv,
    OpticalFlow,
    Filter
};
Q_DECLARE_METATYPE(ProcessType)

//---- Process Item ----//

class TreeItem : public QObject
{
    Q_OBJECT

public:
    explicit TreeItem(QString descrition, QVariant data, TreeItem *parent = 0);
    ~TreeItem();

    TreeItem *child(int number);
    int childCount() const;
    bool insertChildren(int position, int count);
    bool insertChild(int position, TreeItem* child);
    TreeItem *parent();
    bool removeChildren(int position, int count);
    int childNumber() const;

    ProcessType getType() const;
    QVariant getData() const;
    QVariant getDescription() const;
    bool isEditable() const;
    void setData(const QVariant &value);
    void setDescription(const QString description);
    void setIsEditable(const bool value);

signals:
    void dataChanged();

private:
    QVariant itemData;
    QString itemDescription;
    bool editable=true;

    TreeItem *parentItem;
    QList<TreeItem*> childItems;

protected:
    ProcessType itemType;
};

class TreeItemPair : public TreeItem
{
    Q_OBJECT
public:
    explicit TreeItemPair(QString description, QString description1, QString description2, TreeItem* parent);
    void setDataPair(QVariant data1, QVariant data2);
    void getDataPair(int &data1, int &data2);
    void getDataPair(double & data1,double  & data2);

public slots:
    void updateDisplay();
};

class TreeItemBool : public TreeItem
{
    Q_OBJECT
public:
    explicit TreeItemBool(QString description, bool dataIn, TreeItem* parent);
};

class PivItem : public TreeItem
{
public:
    explicit PivItem(TreeItem* parent);
    PivOptions getOptions();
private:
    TreeItemPair * winSize;
    TreeItemPair * winOverlap;
    TreeItemPair * validation;
};

class OptFlowItem : public TreeItem
{
public:
    explicit OptFlowItem(TreeItem* parent);
    OptFlowOptions getOptions();
};

//---- Process Model ----//

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TreeModel(QObject *parent = 0);
    ~TreeModel();

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    void newProcess(int row, ProcessType type);
    void removeProcess(int row);
    QList<TreeItem*> getProcessList();

private:
    void setupModelData(const QStringList &lines, TreeItem *parent);
    TreeItem *getItem(const QModelIndex &index) const;

    TreeItem *rootItem;
};

#endif // PROCESSINGOPTIONS_H
