#include "StickerWrapper.h"
#include <qgraphicsview.h>
#include <qmainwindow.h>
#include <qnamespace.h>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <qpoint.h>

StickerWrapper::StickerWrapper(QWidget* parent)
    : QMainWindow(parent)
{
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

    resize(600, 600);
    updateMask();
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

void StickerWrapper::updateMask()
{
    QPolygonF scenePolygon = proxy->mapToScene(sticker->rect());

    QPolygon windowPolygon;
    for (const QPointF &point : scenePolygon) {
        windowPolygon << point.toPoint();
    }

    setMask(QRegion(windowPolygon));
}

void StickerWrapper::mouseMoveEvent(QMouseEvent *event)
{
    QPointF current = event->position(); 

    if (dragging)
    {
        QPointF pos_ = pos();
        pos_ += current - lastMousePos;

        move(pos_.toPoint());
        event->accept();
    }

    lastMousePos = current;

    // QMainWindow::mouseMoveEvent(event);
}