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

    void startDragging(QPoint start);
    void endDragging();

protected:
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void rotate(float angle);
    void updateMask();

    Sticker* sticker;

    // QGraphicsView* view;
    QGraphicsScene* scene;
    QGraphicsProxyWidget* proxy;

    QPoint lastMousePos;
    bool dragging;

    double angle;
};