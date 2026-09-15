#include "StickerWrapper.h"
#include <qbitmap.h>
#include <qgraphicsview.h>
#include <qmainwindow.h>
#include <qnamespace.h>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <qpainter.h>
#include <qpoint.h>
#include <qpolygon.h>

StickerWrapper::StickerWrapper(QWidget* parent)
    : QMainWindow(parent)
{
    dragging = false;

    setAttribute(Qt::WA_DeleteOnClose); //Enforce deletion
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setMouseTracking(true);
    
    QGraphicsScene* scene = new QGraphicsScene(this); 
    view = new QGraphicsView(scene, this);

    view->setStyleSheet("background: transparent; border: none;");
    view->setRenderHint(QPainter::Antialiasing);
    view->setRenderHint(QPainter::SmoothPixmapTransform);
    setCentralWidget(view);
    
    sticker = new Sticker(this);
    proxy = scene->addWidget(sticker);

    // proxy->setTransformOriginPoint(sticker->rect().center());
    proxy->setRotation(-20.0); 
    resize(600, 600);

    // proxy->setPos(300 - sticker->width()/2, 300 - sticker->height()/2);
    // updateMask();
}

StickerWrapper::~StickerWrapper()
{
}

void StickerWrapper::closeWindow()
{
    close();
}

void StickerWrapper::startDragging()
{
    dragging = true;
}

void StickerWrapper::endDragging()
{
    dragging = false;
}

// void StickerWrapper::updateMask()
// {
//     QPolygonF scenePolygon = proxy->mapToScene(sticker->rect());
    

//     QPolygon windowPolygon = view->mapFromScene(scenePolygon);

//     setMask(scenePolygon.toPolygon());
// }

void StickerWrapper::updateMask()
{
    QRectF localRect = sticker->rect();

    QPolygonF scenePoly = proxy->mapToScene(localRect);
    QPolygon viewPoly = view->mapFromScene(scenePoly);

    QBitmap mask(view->size());
    mask.clear();

    QPainter painter(&mask);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setBrush(Qt::color1);
    painter.setPen(Qt::NoPen);

    painter.drawPolygon(viewPoly);
    painter.end();

    view->setMask(viewPoly);
}

void StickerWrapper::mouseMoveEvent(QMouseEvent *event)
{
    QPoint current = event->position().toPoint(); 

    if ((event->buttons() & Qt::LeftButton) && dragging)
    {
        QPoint pos_ = window()->pos();
        pos_ += current - lastMousePos;

        window()->move(pos_);
        lastMousePos = current;
        event->accept();
        return;
    }

    lastMousePos = current;

    // if (!dragging)
    QMainWindow::mouseMoveEvent(event);
}