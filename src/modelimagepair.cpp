#include "modelimagepair.h"

ModelImagePair::ModelImagePair(QObject *parent) : QAbstractTableModel(parent)
{

}

ModelImagePair::~ModelImagePair()
{
     qDeleteAll(listImPair);
}

int ModelImagePair::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return listImPair.size();
}

int ModelImagePair::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant ModelImagePair::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= listImPair.size() || index.row() < 0)
        return QVariant();

    if ( (role == Qt::FontRole) & (index.row()==activatedRow) ) {
        QFont font;
        font.setBold(true);
        return font;
    }

    if (role == Qt::DisplayRole) {
        ImPairItem *pair = listImPair.at(index.row());
        if (index.column() == 0) {
            return index.row();
        }

        else if (index.column() == 1)
            return pair->imA;
        else if (index.column() == 2)
            return pair->imB;
     }
     return QVariant();
}

QVariant ModelImagePair::headerData(int section, Qt::Orientation orientation, int role) const
 {
     if (role != Qt::DisplayRole)
         return QVariant();

     if (orientation == Qt::Horizontal) {
         switch (section) {
            case 0:
                return "#";
            case 1:
                return "Image A";
            case 2:
                return "Image B";

            default:
                return QVariant();
         }
     }
     return QVariant();
 }

bool ModelImagePair::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position+rows-1);

    for (int row=0; row < rows; row++) {
        ImPairItem *item = new ImPairItem();
        listImPair.insert(position, item);
    }

    endInsertRows();
    return true;
}

bool ModelImagePair::removeRows(int position, int rows, const QModelIndex &index)
 {
     Q_UNUSED(index);
     beginRemoveRows(QModelIndex(), position, position+rows-1);

     for (int row=0; row < rows; ++row) {
         listImPair.removeAt(position);
     }

     endRemoveRows();
     return true;
 }

void ModelImagePair::clear()
{
    beginResetModel();
    listImPair.clear();
    endResetModel();
}

ImPairItem* ModelImagePair::getItem(int row)
{
    if (row<listImPair.size()) {
        activatedRow=row;
        return listImPair.at(row);
    }
    else
        return nullptr;
}

void ModelImagePair::setActivatedItem(int row)
{
    int activatedRowOld=activatedRow;
    activatedRow=row;

    dataChanged(index(activatedRowOld,0),index(activatedRowOld,2),{Qt::FontRole});
    dataChanged(index(activatedRow,0),index(activatedRow,2),{Qt::FontRole});

    emit(selectedItemChanged(row));
}

int ModelImagePair::getSelectedItem() { return activatedRow; }

QList<ImPairItem*> ModelImagePair::getList() { return listImPair; }

int ModelImagePair::getSize() { return listImPair.size(); }
