#pragma once

#include <qobject.h>
#include <qtextedit.h>
#include <qwidget.h>

class StickerEdit : public QTextEdit {
public:
    StickerEdit(QWidget* parent = nullptr);
    ~StickerEdit();

    QString getInnerText() {return innerText;}
    void setInnerText(QString text);

protected:
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;

    void contextMenuEvent(QContextMenuEvent *e) override;

private:
    QString innerText;
    bool isFocused;
};