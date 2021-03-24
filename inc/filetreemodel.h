#ifndef FILETREEMODEL_H
#define FILETREEMODEL_H

#include <QObject>

class FileTreeModel : public QObject
{
    Q_OBJECT
public:
    explicit FileTreeModel(QObject *parent = nullptr);

signals:

public slots:
};

#endif // FILETREEMODEL_H