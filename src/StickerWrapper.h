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

    void startDragging();
    void endDragging();

protected:
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void updateMask();

    Sticker* sticker;

    // QGraphicsView* view;
    QGraphicsScene* scene;
    QGraphicsProxyWidget* proxy;

    QPoint lastMousePos;
    bool dragging;
};