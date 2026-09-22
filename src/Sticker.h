#pragma once

class StickerWrapper;

#include <qframe.h>
#include <qtmetamacros.h>
#include <qwidget.h>
#include <QLineEdit>
#include <QTextEdit>
#include <qtextedit.h>

enum ScalingSide {
    LEFT = 1,
    RIGHT = 2,
    TOP = 4,
    BOTTOM = 8
};

class Sticker : public QFrame
{
public:
    Sticker(StickerWrapper* parent = nullptr);
    ~Sticker();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

    void closeButton_clicked();
    void header_pressed(QMouseEvent* event);
    void header_released(QMouseEvent* event);

    QLineEdit* nameEdit;
    QFrame* header;
    QTextEdit* textEdit;

    StickerWrapper* wrapper;

    uint32_t scalingSide;
    QPointF lastMousePos;
};