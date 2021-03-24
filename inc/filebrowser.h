#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include <QObject>
#include <QDir>
#include <QTreeView>
#include <QFileDialog>
#include <QMimeData>
#include "mainwindow.h"

class MainWindow;
class DragDropQFileSystemModel;

class FileBrowser : public QObject
{
    Q_OBJECT
public:
    explicit FileBrowser(MainWindow *window, QTreeView *treeView);
    void appendHistoryList();
signals:
    void folderChanged(QDir dir);

public slots:
    void updateView();
    void folderUp();
    void folderSelect();
    void folderSelectByIndex(QModelIndex index);
    void folderPrevious();
    void folderNext();
private:
    MainWindow *w;
    QDir workingDir=QDir::current();

    QToolBar* browserBar;
    QAction *previous, *next, *up, *select;
    QLineEdit *dirDisp;
    QToolButton *dirHistory;

    DragDropQFileSystemModel * fileTreeModel;
    QTreeView * folderView;

    QList<QString> historyList;
    int historyIndex;
};

class DragDropQFileSystemModel : public QFileSystemModel
{
    Q_OBJECT
public:
    explicit DragDropQFileSystemModel();
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QStringList mimeTypes() const;
    QMimeData * mimeData(const QModelIndexList &indexes) const;
};

#endif // FILEBROWSER_H
