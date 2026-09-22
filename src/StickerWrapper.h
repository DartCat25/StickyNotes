#pragma once

#include "Sticker.h"
#include <qgraphicsproxywidget.h>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <qgraphicsscene.h>
#include <qinputmethod.h>
#include <qmainwindow.h>

enum StackPos {
    SP_TOP,
    SP_FREE,
    SP_BOTTOM
};

class StickerWrapper : public QGraphicsView
{
public:
    StickerWrapper(StackPos pos = StackPos::SP_TOP);
    ~StickerWrapper();

    void closeWindow();

    void startDragging(QPointF start);
    void endDragging();
    
    void setStackPos(StackPos pos, bool reInit = true);
    StackPos getStackPos();
protected:
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void initWindow();
    void recreateWindow();

    void rotate(float angle);
    void updateMask();
   
    #ifdef Q_OS_LINUX
    void moveRaw(QPoint point);
    void setupNative();
    #endif


    Sticker* sticker;

    
    QGraphicsScene* scene;
    QGraphicsProxyWidget* proxy;
    
    QPointF lastMousePos;
    QPointF pivot;
    bool dragging;
    
    double angle;
    StackPos stackPos;
};