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
#include <qtransform.h>

#define WIDTH 412
#define HEIGHT 412

#define MAX_ANGLE 30.0
#define FORCE 0.1
#define FADE 0.94

// // Constructor / Destructor

StickerWrapper::StickerWrapper(QWidget* parent)
: QGraphicsView(parent)
{
    dragging = false;
    angle = 0;
    
    setAttribute(Qt::WA_DeleteOnClose); //Enforce deletion
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setMouseTracking(true);
    resize(WIDTH, HEIGHT);
    
    scene = new QGraphicsScene(this); 
    scene->setSceneRect(0, 0, WIDTH, HEIGHT);
    
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    
    setScene(scene);

    sticker = new Sticker(this);
    proxy = scene->addWidget(sticker);
    sticker->move(0, 0);

    proxy->setTransformOriginPoint(sticker->rect().center());
    proxy->setRotation(angle);

    proxy->setPos(WIDTH/2 - sticker->width()/2, HEIGHT/2 - sticker->height()/2);
    updateMask();
}

StickerWrapper::~StickerWrapper()
{
}

// // External Events

void StickerWrapper::closeWindow()
{
    close();
}

void StickerWrapper::startDragging(QPointF start)
{
    lastMousePos = start;
    QTransform rotInvert = QTransform();
    rotInvert.rotate(-angle);

    QPointF center = QPointF(WIDTH/2.0, HEIGHT/2.0);
    pivot = rotInvert.map(start - pos() - center) + center;
    dragging = true;
}

void StickerWrapper::endDragging()
{
    dragging = false;
}

// // Inner funcs

void StickerWrapper::rotate(float angle)
{
    proxy->setRotation(angle);
    this->angle = angle;
    updateMask();
}

void StickerWrapper::updateMask()
{
    QPolygonF scenePath = proxy->mapToScene(sticker->rect());   
    QRegion maskRegion(scenePath.toPolygon());
    
    setMask(maskRegion);
}

// // Slot Events

void StickerWrapper::mouseMoveEvent(QMouseEvent *event)
{
    
    if ((event->buttons() & Qt::LeftButton) && dragging)
    {
        QPointF current = event->globalPosition();
        QPointF curPos = pos();
        QPointF vec = current - lastMousePos;

        double newAngle = std::min(std::max(angle * FADE + vec.x() * FORCE, -MAX_ANGLE), MAX_ANGLE);
        
        QPointF center = proxy->mapToScene(proxy->rect().center());
        
        QTransform rotateTransf = QTransform();
        rotateTransf.rotate(newAngle);
        
        curPos = current - rotateTransf.map(pivot - center) - center;
        
        move(curPos.toPoint());
        rotate(newAngle);
        lastMousePos = current;
        event->accept();
        return;
    }

    QGraphicsView::mouseMoveEvent(event);
}