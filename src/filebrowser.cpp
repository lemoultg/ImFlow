#include "filebrowser.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QToolBar>
#include <QFileDialog>
#include <QLineEdit>
#include <QMimeData>
#include <QDropEvent>
#include <iostream>
#include <QTreeView>
#include <QHeaderView>
#include <QToolButton>

#include <QtDebug>

FileBrowser::FileBrowser(MainWindow *window, QTreeView *treeView) : QObject(window), w(window), folderView(treeView)
{
    browserBar=w->addToolBar("File Browser");
    browserBar->setIconSize(QSize(20,20));
    browserBar->setMovable(false);

    //Adding buttons
    previous=browserBar->addAction(QIcon(":/icons/chevron-left"),"");
    previous->setEnabled(false);
    next=browserBar->addAction(QIcon(":/icons/chevron-right"),"");
    next->setEnabled(false);
    up=browserBar->addAction(QIcon(":/icons/arrow-up"),"");
    select=browserBar->addAction(QIcon(":/icons/folder"),"");
    dirDisp = new QLineEdit;
    browserBar->addWidget(dirDisp);
    dirHistory = new QToolButton;
    dirHistory->setIcon(QIcon(":/icons/more-vertical"));
    dirHistory->setPopupMode(QToolButton::InstantPopup);
    browserBar->setStyleSheet("QToolButton::menu-indicator { image:none; }");
    browserBar->addWidget(dirHistory);

    //Setting up the tree view
    fileTreeModel = new DragDropQFileSystemModel();
    fileTreeModel->setRootPath(workingDir.path());
    folderView->setModel(fileTreeModel);

    // Change appearance and hide columns
    folderView->hideColumn(1);
    folderView->hideColumn(2);
    QHeaderView* header = folderView->header();
    header->setStretchLastSection(false);
    header->setSectionResizeMode(0,QHeaderView::Stretch);
    header->setSectionResizeMode(3,QHeaderView::ResizeToContents);
    header->setMinimumSectionSize(10);


    //Allow Drag action
    folderView->setSelectionMode(QAbstractItemView::SingleSelection);
    folderView->setDragEnabled(true);
    folderView->setAcceptDrops(false);

    //Controlling the appearance
    folderView->setAnimated(false);
    folderView->setIndentation(20);
    folderView->setSortingEnabled(true);

    //Connect buttons to actions
    connect(up,SIGNAL(triggered()),this,SLOT(folderUp()));
    connect(select,SIGNAL(triggered()),this,SLOT(folderSelect()));
    connect(folderView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(folderSelectByIndex(QModelIndex)));
    connect(previous,SIGNAL(triggered()),this,SLOT(folderPrevious()));
    connect(next,SIGNAL(triggered()),this,SLOT(folderNext()));

    historyList.append(workingDir.path());
    historyIndex=0;
    updateView();

    //fileTreeModel->data()
}

void FileBrowser::folderUp()
{
    workingDir.cdUp();
    appendHistoryList();
    updateView();
}

void FileBrowser::folderSelect()
{
    workingDir=QFileDialog::getExistingDirectory(w,"Select working directory","/",QFileDialog::ShowDirsOnly);
    appendHistoryList();
    updateView();
}

void FileBrowser::folderSelectByIndex(QModelIndex index)
{
    if (fileTreeModel->isDir(index))
        workingDir=fileTreeModel->filePath(index);
    appendHistoryList();
    updateView();
}

void FileBrowser::folderPrevious()
{
    historyIndex--;
    workingDir=historyList[historyIndex];
    updateView();
}

void FileBrowser::folderNext()
{
    historyIndex++;
    workingDir=historyList[historyIndex];
    updateView();
}

void FileBrowser::updateView()
{
    up->setEnabled(workingDir!=QDir::root());
    previous->setEnabled(historyIndex!=0);
    next->setEnabled(historyIndex<(historyList.size()-1));

    fileTreeModel->setRootPath(workingDir.path());
    folderView->setRootIndex(fileTreeModel->index(workingDir.path()));
    dirDisp->setText(workingDir.path());

    emit folderChanged(workingDir);
}

void FileBrowser::appendHistoryList()
{
    while (historyList.size()>(historyIndex+1))
        historyList.removeLast();
    historyList.append(workingDir.path());
    historyIndex++;
}

DragDropQFileSystemModel::DragDropQFileSystemModel() : QFileSystemModel(nullptr)
{

}

Qt::ItemFlags DragDropQFileSystemModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QFileSystemModel::flags(index);

    if (index.isValid() & (this->isDir(index)==false) )
        return Qt::ItemIsDragEnabled | defaultFlags;
    else
        return Qt::ItemIsEnabled;
}

QStringList DragDropQFileSystemModel::mimeTypes() const
{
    QStringList types;
    types << "application/vnd.text.list";
    return types;
}

QMimeData * DragDropQFileSystemModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for (int i=0; i<indexes.size(); i+=2) {
        if (indexes[i].isValid()) {
            QString text = this->filePath(indexes[i]);
            stream << text;
        }
    }

    mimeData->setData("application/vnd.text.list", encodedData);
    return mimeData;
}
