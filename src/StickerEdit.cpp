#include "StickerEdit.h"
#include <qevent.h>
#include <qtextedit.h>

StickerEdit::StickerEdit(QWidget* parent) : QTextEdit(parent)
{
    innerText = "";
    isFocused = false;
}

StickerEdit::~StickerEdit()
{}

void StickerEdit::focusInEvent(QFocusEvent* e)
{
    setPlainText(innerText);
    isFocused = true;
    QTextEdit::focusInEvent(e);
}

void StickerEdit::focusOutEvent(QFocusEvent* e)
{
    setMarkdown(innerText);
    isFocused = false;
    QTextEdit::focusOutEvent(e);
}

void StickerEdit::setInnerText(QString text)
{
    innerText = text;
    
    if (isFocused)
        setMarkdown(innerText);
    else
        setPlainText(innerText);
}