#pragma once

#include "Sticker.h"
#include <qgraphicsproxywidget.h>
#include <qinputmethod.h>
#include <qmainwindow.h>

class StickerWrapper : public QMainWindow
{
public:
    StickerWrapper(QWidget* parent = nullptr);
    ~StickerWrapper();

    void closeWindow();
private:
    Sticker* sticker;
    QGraphicsProxyWidget* proxy;
};