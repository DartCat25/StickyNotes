#pragma once
#include <qobject.h>
#include <qtextedit.h>
#include <qwidget.h>

class StickerEdit : public QTextEdit {
public:
    StickerEdit(QWidget* parent = nullptr);
    ~StickerEdit();

protected:
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;

private:
    QString innerText;
};