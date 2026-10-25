#pragma once

#include "StickerStorage.h"
#include "StickerWrapper.h"
#include <QObject>
#include <QPoint>

class StickerManager : public QObject {
    Q_OBJECT

public:
    StickerManager();
    ~StickerManager();

private slots:
    void onTransformChanged(
        int32_t x, int32_t y,
        uint32_t w, uint32_t h,
        float rotation);

    void onTextChanged(QString text);
    void onColorChanged(QColor color);
    void onStackPosChanged(StackPos pos);
    void onCloseRequested();

private:
    void connectSticker(StickerWrapper* sticker);
    void createCloner(QPoint pos);
    void materializeCloner(StickerWrapper* sticker);
    QPoint defaultClonerPos();

    StickerWrapper* cloner;
    StickerStorage* storage;
};
