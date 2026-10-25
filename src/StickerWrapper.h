#pragma once

#include "Sticker.h"
#include <qgraphicsproxywidget.h>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <qgraphicsscene.h>
#include <qinputmethod.h>
#include <qmainwindow.h>
#include "structs.h"

class StickerWrapper : public QGraphicsView
{
    Q_OBJECT

public:
    StickerWrapper(
        uint32_t id,
        StackPos pos = StackPos::SP_FREE,
        QWidget* parent = nullptr);
    StickerWrapper(
        uint32_t id,
        const StickerData* data,
        QWidget* parent = nullptr);
    ~StickerWrapper();

    void applyData(const StickerData* data);
    StickerData getData();

    void closeWindow();

    void startDragging(QPointF start);
    void endDragging();
    
    void updateMask();

    #ifdef Q_OS_LINUX
    void moveRaw(QPoint point);
    #endif
    
    uint32_t getId() {return id;}
    void setId(uint32_t id) {this->id = id;}

    void setStackPos(StackPos pos, bool reInit = true);
    StackPos getStackPos();
    
    void setAngle(float angle);
    float getAngle();

    void setSize(uint32_t w, uint32_t h);

    void setBackgroundColor(QColor color);
    QColor getBackgroundColor();

    void setText(QString text);
    QString getText();

    void notifyTransformChanged();
    void notifyTextChanged();

signals:
    void transformChanged(
        int32_t x, int32_t y,
        uint32_t w, uint32_t h,
        float rotation);
    void textChanged(QString text);
    void colorChanged(QColor color);
    void stackPosChanged(StackPos pos);
    void closeRequested();

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void closeEvent(QCloseEvent* event) override;

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

    uint32_t id;
};