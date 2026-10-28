#pragma once

#include <qobject.h>
#include <qtextedit.h>
#include <qwidget.h>
#include <QTextBlock>

class StickerEdit : public QTextEdit {
public:
    StickerEdit(QWidget* parent = nullptr);
    ~StickerEdit();

    QString getInnerText() {return innerText;}
    void setInnerText(QString text);

protected:
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;

    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;

    void contextMenuEvent(QContextMenuEvent *e) override;

private:
    QTextBlock markerBlockAt(const QPointF& pos) const;
    int checkboxPosition(QTextBlock block) const;
    void toggleCheckbox(QTextBlock block);

    QString innerText;
    bool isFocused;
    bool checkboxPressed;
};
