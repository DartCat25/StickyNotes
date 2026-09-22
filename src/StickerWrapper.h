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
    StickerWrapper(
        bool isCloner = false,
        StackPos pos = StackPos::SP_FREE,
        QWidget* parent = nullptr);
    ~StickerWrapper();

    void closeWindow();

    void startDragging(QPointF start);
    void endDragging();
    
    void setStackPos(StackPos pos, bool reInit = true);
    StackPos getStackPos();
    void updateMask();

    #ifdef Q_OS_LINUX
    void moveRaw(QPoint point);
    #endif
protected:
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void initWindow();
    void recreateWindow();
    
    #ifdef Q_OS_LINUX
    void setupNative();
    #endif

    void rotate(float angle);

    Sticker* sticker;
    
    QGraphicsScene* scene;
    QGraphicsProxyWidget* proxy;
    
    QPointF lastMousePos;
    QPointF pivot;
    bool dragging;
    
    double angle;
    StackPos stackPos;

    bool isCloner;
};