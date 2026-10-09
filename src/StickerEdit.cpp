#include "StickerEdit.h"
#include <qevent.h>
#include <qtextedit.h>

StickerEdit::StickerEdit(QWidget* parent) : QTextEdit(parent)
{
    innerText = "";
}

StickerEdit::~StickerEdit()
{}

void StickerEdit::focusInEvent(QFocusEvent* e)
{
    setPlainText(innerText);
    QTextEdit::focusInEvent(e);
}

void StickerEdit::focusOutEvent(QFocusEvent* e)
{
    innerText = toPlainText();
    setMarkdown(innerText);
    QTextEdit::focusOutEvent(e);
}