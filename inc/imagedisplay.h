#ifndef IMAGEDISPLAY_H
#define IMAGEDISPLAY_H

#include <QObject>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QDir>

#include <opencv2/core.hpp>

using namespace cv;

class QGraphicsImageItem;
class QGraphicsArrowItem;

class ImageDisplay : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ImageDisplay(QWidget *parent = Q_NULLPTR);
    ~ImageDisplay();
    void setImage(Mat im);
    void setImageScale(double scale);
    QGraphicsArrowItem *addVector(int x, int y, float length, float angle);
    void clearVector(QList<QGraphicsArrowItem*> arrowList);
    void updateImage();

public slots:
    void wheelEvent(QWheelEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

    void zoomIn();
    void zoomOut();
    void zoomOrig();
    void zoomFit();

signals:
    void zoomUpdated(double zoom);
    void mouseMoved(int posX,int posY);
    void dropEventReceived(QDir fileName);

private:
    QGraphicsScene *scene;
    //QGraphicsPixmapItem *imPaint;
    QGraphicsImageItem *imPaint;

    double zoom=1;

    int width, height;
};

// QGraphicsImageItem class inherits QGraphicsItem
class QGraphicsImageItem : public QGraphicsItem
{
public:
    explicit QGraphicsImageItem();
    ~QGraphicsImageItem();
    void setImage(Mat imColor);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);
    QRectF boundingRect() const;

private:
    QImage qImDisp;
};

// QGraphicsArrowItem class inherits QGraphicsItem
class QGraphicsArrowItem : public QGraphicsItem
{
public:
    explicit QGraphicsArrowItem();
    ~QGraphicsArrowItem();
    void setLength(float length);
    void setColor(QColor color);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);
    QRectF boundingRect() const;

private:
    QPainterPath path;
    QPen pen;
    qreal length;
};


#endif // IMAGEDISPLAY_H
