#include "StickerEdit.h"
#include <qaction.h>
#include <qclipboard.h>
#include <qevent.h>
#include <qguiapplication.h>
#include <qmenu.h>
#include <qnamespace.h>
#include <qtextedit.h>
#include <qwindowdefs.h>
#include <QClipboard>

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
    innerText = toPlainText();
    setMarkdown(innerText);
    isFocused = false;
    QTextEdit::focusOutEvent(e);
}

//Remove base event
void StickerEdit::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu = QMenu();

    QAction* copy = menu.addAction("Copy");

    QAction *selectedAction = menu.exec(e->globalPos());

    if (selectedAction == copy)
    {
        QClipboard* clip = QGuiApplication::clipboard();

        clip->setText(innerText);
        if (clip->supportsSelection())
        {
            clip->setText(innerText, QClipboard::Selection);
        }
    }
}

void StickerEdit::setInnerText(QString text)
{
    innerText = text;
    
    if (isFocused)
        setMarkdown(innerText);
    else
        setPlainText(innerText);
}