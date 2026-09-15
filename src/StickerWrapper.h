#pragma once

#include "Sticker.h"
#include <qgraphicsproxywidget.h>
#include <QGraphicsView>
#include <qinputmethod.h>
#include <qmainwindow.h>

class StickerWrapper : public QMainWindow
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

    QGraphicsView* view;
    QGraphicsProxyWidget* proxy;

    QPointF lastMousePos;
    bool dragging;
};