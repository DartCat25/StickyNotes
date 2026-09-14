#include "StickerWrapper.h"
#include <qgraphicsview.h>
#include <qnamespace.h>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>

StickerWrapper::StickerWrapper(QWidget* parent)
    : QMainWindow(parent)
{
    setAttribute(Qt::WA_DeleteOnClose); //Enforce deletion
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    
    QGraphicsScene* scene = new QGraphicsScene(this); 
    QGraphicsView* view = new QGraphicsView(scene, this);

    view->setStyleSheet("background: transparent; border: none;");
    view->setRenderHint(QPainter::Antialiasing);
    view->setRenderHint(QPainter::SmoothPixmapTransform);
    setCentralWidget(view);
    
    sticker = new Sticker(this);
    proxy = scene->addWidget(sticker);

    resize(600, 600);
}

StickerWrapper::~StickerWrapper()
{
    // delete sticker;
}

void StickerWrapper::closeWindow()
{
    close();
}