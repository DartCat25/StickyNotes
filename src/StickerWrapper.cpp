#include "StickerWrapper.h"
#include <qbitmap.h>
#include <qgraphicsview.h>
#include <qmainwindow.h>
#include <qnamespace.h>
#include <QGraphicsProxyWidget>
#include <qpainter.h>
#include <qpoint.h>
#include <qpolygon.h>
#include <QWindow>

#define WIDTH 412
#define HEIGHT 412


StickerWrapper::StickerWrapper(QWidget* parent)
: QGraphicsView(parent)
{
    dragging = false;
    
    setAttribute(Qt::WA_DeleteOnClose); //Enforce deletion
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    setMouseTracking(true);
    resize(WIDTH, HEIGHT);
    
    scene = new QGraphicsScene(this); 
    scene->setSceneRect(0, 0, WIDTH, HEIGHT);
    
    setStyleSheet("background: transparent; border: none;");
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    
    setScene(scene);

    sticker = new Sticker(this);
    proxy = scene->addWidget(sticker);
    sticker->move(0, 0);

    proxy->setTransformOriginPoint(sticker->rect().center());
    // proxy->setPos(100, 100);
    proxy->setRotation(30.0);

    proxy->setPos(WIDTH/2 - sticker->width()/2, HEIGHT/2 - sticker->height()/2);
    updateMask();
}

StickerWrapper::~StickerWrapper()
{
}

void StickerWrapper::closeWindow()
{
    close();
}

void StickerWrapper::startDragging(QPoint start)
{
    startMousePos = start;
    dragging = true;
}

void StickerWrapper::endDragging()
{
    dragging = false;
}

// void StickerWrapper::updateMask()
// {
//     QPolygonF scenePolygon = proxy->mapToScene(proxy->rect());
    

//     QPolygon windowPolygon = view->mapFromScene(scenePolygon);

//     setMask(scenePolygon.toPolygon());
// }

void StickerWrapper::updateMask()
{
    QRectF localRect = sticker->rect();

    QPolygonF scenePath = proxy->mapToScene(localRect);
    
    // Переводим путь со сцены в координаты самого окна (Viewport)
    QPolygonF viewPath = mapFromScene(scenePath);
    
    // Создаем регион (маску) из полученного повернутого контура
    QRegion maskRegion(scenePath.toPolygon());
    
    // Применяем маску к окну. Все, что вне этого повернутого прямоугольника, 
    // станет невидимым и кликабельным насквозь.
    setMask(maskRegion);
}

void StickerWrapper::mouseMoveEvent(QMouseEvent *event)
{
    QPoint current = event->globalPosition().toPoint(); 

    if ((event->buttons() & Qt::LeftButton) && dragging)
    {
        QPoint pos_ = pos();
        pos_ += current - startMousePos;

        move(pos_);
        event->accept();
        return;
    }

    // if (!dragging)
    QGraphicsView::mouseMoveEvent(event);
}