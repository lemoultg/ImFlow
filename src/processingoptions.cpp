#include "processingoptions.h"
#include "ui_processingoptions.h"

#include "optflow.h"
#include "pivengine.h"

#include <QDebug>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <iostream>

using namespace cv;

ProcessingOptions::ProcessingOptions(MainWindow *parent) :
    QWidget(parent),
    ui(new Ui::ProcessingOptions),
    mainWindow(parent)
{
    ui->setupUi(this);

    ui->addSelector->addItem("Optical Flow",ProcessType::OpticalFlow);
    ui->addSelector->addItem("PIV",ProcessType::Piv);
    ui->addSelector->addItem("Filter",ProcessType::Filter);

    model = new TreeModel(this);
    ui->treeView->setModel(model);

    QHeaderView* hHeader = ui->treeView->header();
    hHeader->setSectionResizeMode(0,QHeaderView::Stretch);
    hHeader->setSectionResizeMode(1,QHeaderView::ResizeToContents);

    //Connect add and remove buttons
    connect(ui->addButton,SIGNAL(clicked(bool)),this,SLOT(addProcessing()));
    connect(ui->removeButton,SIGNAL(clicked(bool)),this,SLOT(removeProcessing()));

    //Trigger processing
    //connect(ui->processButton,SIGNAL(clicked(bool)),mainWindow,SLOT(processPair()));
    //Trigger processing when auto is checked
    //connect(model,SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),this,SLOT(autoProcess()));
    //connect(this,SIGNAL(autoProcessSignal()),mainWindow,SLOT(processPair()));

}

ProcessingOptions::~ProcessingOptions()
{
    delete ui;
}

void ProcessingOptions::addProcessing()
{
    QVariant selectedProcess = ui->addSelector->currentData();
    if (selectedProcess.isValid()) {
        ProcessType processType = selectedProcess.value<ProcessType>();

        int row;
        QModelIndex itemIdx = ui->treeView->currentIndex();
        if (!itemIdx.isValid())
            row=0;
        else {
            // Finding top level item corresponding to selected item
            while (itemIdx.parent().isValid())
                itemIdx=itemIdx.parent();
            row=itemIdx.row()+1;
        }
        model->newProcess(row,processType);
    }
}

void ProcessingOptions::removeProcessing()
{

    QModelIndex itemIdx = ui->treeView->currentIndex();
    if (!itemIdx.isValid())
        return;

    // Finding top level item corresponding to selected item
    while (itemIdx.parent().isValid()) {
        itemIdx=itemIdx.parent();
    }

    model->removeProcess(itemIdx.row());
}

QList<TreeItem *> ProcessingOptions::getProcessList()
{
    return model->getProcessList();
}

void ProcessingOptions::autoProcess()
{
    if (ui->autoProcessButton->isChecked())
        emit(autoProcessSignal());
}

//---- Process Item ----//

TreeItem::TreeItem(QString descrition, QVariant data, TreeItem *parent) : QObject(parent)
{
    parentItem = parent;
    itemData = data;
    itemDescription = descrition;
}

TreeItem::~TreeItem()
{
    qDeleteAll(childItems);
}

TreeItem *TreeItem::child(int number)
{
    return childItems.value(number);
}

int TreeItem::childCount() const
{
    return childItems.count();
}

int TreeItem::childNumber() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

    return 0;
}

QVariant TreeItem::getDescription() const
{
    return itemDescription;
}

ProcessType TreeItem::getType() const
{
    return itemType;
}

QVariant TreeItem::getData() const
{
    return itemData;
}

bool TreeItem::insertChildren(int position, int count)
{
    if (position < 0 || position > childItems.size())
        return false;

    for (int row = 0; row < count; ++row) {
        TreeItem *item = new TreeItem("",QVariant(), this);
        childItems.insert(position, item);
    }

    return true;
}

bool TreeItem::insertChild(int position, TreeItem* child)
{
    if (position < 0 || position > childItems.size())
        return false;

    childItems.insert(position, child);

    return true;
}

TreeItem *TreeItem::parent()
{
    return parentItem;
}

bool TreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete childItems.takeAt(position);

    return true;
}

void TreeItem::setData(const QVariant &value)
{
    itemData = value;

    emit dataChanged();
}

void TreeItem::setDescription(const QString description)
{
    itemDescription=description;
}

void TreeItem::setIsEditable(const bool value) {editable=value;}

bool TreeItem::isEditable() const {return editable;}

TreeItemPair::TreeItemPair(QString description, QString description1, QString description2, TreeItem* parent) :
    TreeItem(description,QString(),parent)
{
    TreeItem* firstItem = new TreeItem(description1,QVariant(),this);
    TreeItem* secondItem = new TreeItem(description2,QVariant(),this);
    insertChild(0,firstItem);
    insertChild(1,secondItem);

    setIsEditable(false);

    connect(firstItem,SIGNAL(dataChanged()),this,SLOT(updateDisplay()));
    connect(secondItem,SIGNAL(dataChanged()),this,SLOT(updateDisplay()));
}

void TreeItemPair::updateDisplay()
{
    setData(QString("(%1,%2)").arg(child(0)->getData().toInt()).arg(child(1)->getData().toInt()) );
}

void TreeItemPair::setDataPair(QVariant data1,QVariant data2)
{
    child(0)->setData(data1);
    child(1)->setData(data2);
}

void TreeItemPair::getDataPair(int & data1,int  & data2)
{
    data1 = child(0)->getData().toInt();
    data2 = child(1)->getData().toInt();
}

void TreeItemPair::getDataPair(double & data1,double  & data2)
{
    data1 = child(0)->getData().toDouble();
    data2 = child(1)->getData().toDouble();
}

PivItem::PivItem(TreeItem* parent) : TreeItem("PIV",QVariant(),parent)
{
    itemType = ProcessType::Piv;

    setIsEditable(false);

    insertChild(0,new TreeItem("Use previous flow",false,this));

    winSize = new TreeItemPair("Window Size","x","y",this);
    winSize->setDataPair(32,32);
    insertChild(1,winSize);

    winOverlap = new TreeItemPair("Window Overlap","x","y",this);
    winOverlap->setDataPair(0,0);
    insertChild(2,winOverlap);

    validation = new TreeItemPair("Validation","Peak Ratio","Mean Value",this);
    validation->setDataPair(1.,0.);
    insertChild(3,validation);
}

PivOptions PivItem::getOptions()
{
    PivOptions res;

    winSize->getDataPair(res.xWinLen,res.yWinLen);
    winOverlap->getDataPair(res.xWinOver,res.yWinOver);
    validation->getDataPair(res.peakRatio,res.meanValue);

    res.usePrevious=child(0)->getData().toBool();

    return res;
}

OptFlowItem::OptFlowItem(TreeItem* parent) : TreeItem("Optical Flow",QVariant(),parent)
{
    itemType = ProcessType::OpticalFlow;

    setIsEditable(false);

    insertChild(0,new TreeItem("numLevels",4,this));
    insertChild(1,new TreeItem("pyrScale",0.5,this));
    insertChild(2,new TreeItem("fastPyramids",true,this));
    insertChild(3,new TreeItem("winSize",16,this));
    insertChild(4,new TreeItem("numIters",5,this));
    insertChild(5,new TreeItem("polyN",5,this));
    insertChild(6,new TreeItem("polySigma",1.1,this));
    insertChild(7,new TreeItem("Use Initial Flow",true,this));
}

OptFlowOptions OptFlowItem::getOptions()
{
    OptFlowOptions res;

    res.numLevels=child(0)->getData().toInt();
    res.pyrScale=child(1)->getData().toDouble();
    res.fastPyramids=child(2)->getData().toBool();
    res.winSize=child(3)->getData().toInt();
    res.numIters=child(4)->getData().toInt();
    res.polyN=child(5)->getData().toInt();
    res.polySigma=child(6)->getData().toDouble();
    res.flags=child(7)->getData().toBool() ? OPTFLOW_USE_INITIAL_FLOW : 0;

    return res;
}

//---- Process Model ----//

TreeModel::TreeModel(QObject *parent) : QAbstractItemModel(parent)
{
    rootItem = new TreeItem("root",0);
}

TreeModel::~TreeModel()
{
    delete rootItem;
}

int TreeModel::columnCount(const QModelIndex & /* parent */) const
{
    return 2;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    TreeItem *item = getItem(index);
    if (index.column()==0)
        return item->getDescription();
    else if (index.column()==1)
        return item->getData();
    else
        return QVariant();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    if (index.column()==0)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (getItem(index)->isEditable())
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

TreeItem *TreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section==0)
            return "Description";
        else if (section==1)
            return "Value";
    }

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    TreeItem *parentItem = getItem(parent);

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem = getItem(parent);

    return parentItem->childCount();
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    TreeItem *item = getItem(index);
    item->setData(value);
    emit dataChanged(index, index, {role});

    return true;
}

void TreeModel::newProcess(int row, ProcessType type)
{
    beginInsertRows(QModelIndex(), row, row);
    switch (type) {
    case Piv:
        rootItem->insertChild(row,new PivItem(rootItem));
        break;
    case OpticalFlow:
        rootItem->insertChild(row,new OptFlowItem(rootItem));
        break;
    case Filter:
        break;
    }
    endInsertRows();
}

void TreeModel::removeProcess(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    rootItem->removeChildren(row,1);
    endRemoveRows();
}

QList<TreeItem *> TreeModel::getProcessList()
{
    QList<TreeItem*> list;
    for (int i=0; i<rootItem->childCount(); i++) {
        list.append(rootItem->child(i));
    }
    return list;
}














