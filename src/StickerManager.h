#pragma once

#include "StickerStorage.h"
#include "StickerWrapper.h"
#include <QObject>
#include <QPoint>
#include <QSystemTrayIcon>
#include <QMenu>

struct Config;

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

    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onOpenStickersDir();
    void onOpenConfig();

private:
    void connectSticker(StickerWrapper* sticker);
    void createCloner(QPoint pos);
    void materializeCloner(StickerWrapper* sticker);
    void persistSticker(StickerWrapper* sticker);
    void createStickerAt(QPoint pos);
    QPoint defaultClonerPos();
    QPoint screenCenter();

    StickerWrapper* cloner;
    StickerStorage* storage;
    const Config* config;
    bool shuttingDown;

    QMenu* menu;
};
