#pragma once

class StickerWrapper;

#include <qframe.h>
#include <qtmetamacros.h>
#include <qwidget.h>
#include <QLineEdit>

class Sticker : public QFrame
{
public:
    Sticker(StickerWrapper* parent = nullptr);
    ~Sticker();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void closeButton_clicked();
    void header_pressed();
    void header_released();

    QLineEdit* nameEdit;
    QFrame* header;

    StickerWrapper* wrapper;
};