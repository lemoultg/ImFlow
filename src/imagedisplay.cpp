#include "imagedisplay.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QWheelEvent>
#include <QGraphicsPixmapItem>
#include <QPointF>
#include <QDir>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QString>

#include <QtDebug>


#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>

using namespace cv;

ImageDisplay::ImageDisplay(QWidget *parent) : QGraphicsView(parent)
{
    scene = new QGraphicsScene(this);
    setScene(scene);
    show();

    imPaint = new QGraphicsImageItem;
    scene->addItem(imPaint);

    setMouseTracking(true);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setAcceptDrops(true);
}

void ImageDisplay::setImage(Mat im)
{
    imPaint->setImage(im);
    scene->setSceneRect(imPaint->boundingRect());
    width=im.cols;
    height=im.rows;
}

void ImageDisplay::setImageScale(double scale)
{
    imPaint->setScale(scale);
}

void ImageDisplay::zoomIn()
{
    scale(1.05,1.05);
    zoom*=1.05;
    emit zoomUpdated(zoom);
}

void ImageDisplay::zoomOut()
{
    scale(0.95,0.95);
    zoom*=0.95;
    emit zoomUpdated(zoom);
}

void ImageDisplay::zoomOrig()
{
    resetTransform();
    zoom=1;
    emit zoomUpdated(zoom);
}

void ImageDisplay::zoomFit()
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    fitInView(imPaint,Qt::KeepAspectRatio);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QPoint sizeInPixel = mapFromScene(imPaint->boundingRect().width(),imPaint->boundingRect().height());
    zoom= (double)sizeInPixel.x()/width;
    emit zoomUpdated(zoom);
}

void ImageDisplay::updateImage()
{
    imPaint->update(imPaint->boundingRect());
}

QGraphicsArrowItem* ImageDisplay::addVector(int x, int y, float length, float angle)
{
    QGraphicsArrowItem* arrow = new QGraphicsArrowItem;
    arrow->setLength(length);
    arrow->setPos(QPointF(x,y)+QPointF(0.5,0.5));
    arrow->setRotation(angle);
    scene->addItem(arrow);
    return arrow;
}

void ImageDisplay::clearVector(QList<QGraphicsArrowItem*> arrowList)
{
    QGraphicsArrowItem* arrow;
    foreach (arrow,arrowList) {
        scene->removeItem(arrow);
    }
}

void ImageDisplay::wheelEvent(QWheelEvent *event)
// Accept wheelEvent and change zoom accordingly
{
    if (event->angleDelta().y()>0)
        zoomIn();
    else if (event->angleDelta().y()<0)
        zoomOut();
    emit zoomUpdated(zoom);
    event->accept();
}

void ImageDisplay::mouseMoveEvent(QMouseEvent *event)
{
    QPointF mousePos = this->mapToScene(event->pos());

    int posX = (int)mousePos.x();
    int posY = (int)mousePos.y();

    if ( (posX>=0) & (posX<width) & (posY>=0) & (posY<height) ) {
        emit mouseMoved(posX,posY);
    }
    QGraphicsView::mouseMoveEvent(event);
}

void ImageDisplay::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/vnd.text.list") | event->mimeData()->hasFormat("text/plain")) {
        event->acceptProposedAction();
    }
}

void ImageDisplay::dragMoveEvent(QDragMoveEvent *event)
// Needed because QGraphicsScene provides a default implementation of dragMoveEvent that checks if
// there are items on the scene under the cursor that accept drop events. If not, the event will be
// ignored, and you won't be allowed to drop.
{
     Q_UNUSED(event);
}

void ImageDisplay::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/vnd.text.list")) {
        QByteArray encodedData = event->mimeData()->data("application/vnd.text.list");
        QDataStream stream(&encodedData, QIODevice::ReadOnly);
        QStringList newItems;
        int rows = 0;
        while (!stream.atEnd()) {
            QString text;
            stream >> text;
            newItems << text;
            ++rows;
        }
        emit dropEventReceived( QDir(newItems.last()) );
    }
    else if (event->mimeData()->hasFormat("text/plain")) {
        QString input=event->mimeData()->text();
        int nFile = input.count("file://");
        if (nFile==1) {
            input.remove("file://");
            input.replace("%20"," ");
            input.remove("\r\n");
            emit dropEventReceived(QDir(input));
        }
    }

    //textBrowser->setPlainText(event->mimeData()->text());
    //mimeTypeCombo->clear();
    //mimeTypeCombo->addItems(event->mimeData()->formats());

    event->acceptProposedAction();
}

ImageDisplay::~ImageDisplay()
{
    delete imPaint;
    delete scene;
}

//------------------------- QGraphicsImageItem -------------------------//

QGraphicsImageItem::QGraphicsImageItem() : QGraphicsItem(nullptr)
{
    qImDisp=QImage();
}

QGraphicsImageItem::~QGraphicsImageItem()
{

}

void QGraphicsImageItem::setImage(Mat imColor)
{

    qImDisp=QImage((uchar*)imColor.data, imColor.cols, imColor.rows, imColor.step, QImage::Format_RGB888);
    prepareGeometryChange();
}

void QGraphicsImageItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED( option )
    Q_UNUSED( widget )
    painter->drawImage(QPoint(0,0),qImDisp);
}

QRectF QGraphicsImageItem::boundingRect() const
{
    return QRectF(qImDisp.rect());
}

//------------------------------- Arrow -------------------------------//

QGraphicsArrowItem::QGraphicsArrowItem() : QGraphicsItem(nullptr)
{
    pen = QPen(Qt::red, 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
}

QGraphicsArrowItem::~QGraphicsArrowItem() {}

void QGraphicsArrowItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED( option )
    Q_UNUSED( widget )


    painter->setPen(pen);
    painter->setBrush(Qt::red);

    painter->drawPath(path);
}

void QGraphicsArrowItem::setLength(float lengthIn)
{
    length=lengthIn;

    path = QPainterPath();

    QPolygonF arrowHead;
    arrowHead << QPointF(.5*length,0) << QPointF(.25*length,0.1*length) << QPointF(.25*length,-0.1*length);
    path.addPolygon(arrowHead);
    path.closeSubpath();
    path.addRect(-.5*length,-0.025*length,0.75*length,0.05*length);
}

void QGraphicsArrowItem::setColor(QColor color)
{
    pen.setColor(color);
}

QRectF QGraphicsArrowItem::boundingRect() const
{
    return path.boundingRect();
}

