#pragma once

#include "Sticker.h"
#include <qgraphicsproxywidget.h>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <qgraphicsscene.h>
#include <qinputmethod.h>
#include <qmainwindow.h>

class StickerWrapper : public QGraphicsView
{
public:
    StickerWrapper(QWidget* parent = nullptr);
    ~StickerWrapper();

    void closeWindow();

    void startDragging(QPointF start);
    void endDragging();

protected:
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void rotate(float angle);
    void updateMask();

    Sticker* sticker;

    QGraphicsScene* scene;
    QGraphicsProxyWidget* proxy;

    QPointF lastMousePos;
    QPointF pivot;
    bool dragging;

    double angle;
};