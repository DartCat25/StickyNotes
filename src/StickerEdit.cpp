#include "StickerEdit.h"
#include "Sticker.h"
#include <qabstracttextdocumentlayout.h>
#include <qaction.h>
#include <qclipboard.h>
#include <qdesktopservices.h>
#include <qevent.h>
#include <qguiapplication.h>
#include <qmenu.h>
#include <qnamespace.h>
#include <qscrollbar.h>
#include <qtextcursor.h>
#include <qtextdocument.h>
#include <qurl.h>
#include <qwindowdefs.h>
#include <QClipboard>
#include <QRegularExpression>

namespace {

// A task list item of the source text. The position points at the character
// between the brackets, so the text keeps its original formatting.
struct Checkbox {
    int position;
    QString text;
};

QList<Checkbox> sourceCheckboxes(const QString& text)
{
    static const QRegularExpression line(
        "^(?:[ \\t]*>[ \\t]?)*[ \\t]*(?:[-*+]|\\d+[.)])[ \\t]+\\[([ xX])\\](?:[ \\t]|$)");

    QList<Checkbox> result;
    bool fence = false;
    int start = 0;

    while (start <= text.size())
    {
        int end = text.indexOf('\n', start);

        if (end < 0)
            end = text.size();

        QString current = text.mid(start, end - start);
        QString trimmed = current.trimmed();

        // A fenced code block is shown as it is, it has no checkboxes.
        if (trimmed.startsWith("```") || trimmed.startsWith("~~~"))
            fence = !fence;
        else if (!fence)
        {
            QRegularExpressionMatch match = line.match(current);

            if (match.hasMatch())
            {
                int position = start + match.capturedStart(1);
                int bracket = current.indexOf(']', match.capturedStart(1));

                result.append(Checkbox{position, current.mid(bracket + 1).trimmed()});
            }
        }

        start = end + 1;
    }

    return result;
}

// Qt marks paragraphs after a task list as well, so only list items count.
bool isCheckbox(QTextBlock block)
{
    if (!block.isValid() || !block.textList())
        return false;

    QTextBlockFormat format = block.blockFormat();

    return format.hasProperty(QTextFormat::BlockMarker)
        && format.marker() != QTextBlockFormat::MarkerType::NoMarker;
}

}

StickerEdit::StickerEdit(QWidget* parent) : QTextEdit(parent)
{
    innerText = "";
    isFocused = false;
    checkboxPressed = false;
    linkPressed = false;

    // Clicks no longer take the focus by themselves, see mousePressEvent.
    setFocusPolicy(Qt::TabFocus);
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

    if (parent())
    {
        Sticker* sticker = (Sticker*)parent();

        sticker->notifyTextChanged();
    }
    QTextEdit::focusOutEvent(e);
}

// The layout is drawn without the scroll offset, the same way the position is
// passed to the text control by QTextEdit.
QTextBlock StickerEdit::markerBlockAt(const QPointF& pos) const
{
    QPointF documentPos = pos + QPointF(
        horizontalScrollBar()->value(),
        verticalScrollBar()->value());

    return document()->documentLayout()->blockWithMarkerAt(documentPos);
}

int StickerEdit::checkboxPosition(QTextBlock block) const
{
    QList<QTextBlock> blocks;

    for (QTextBlock b = document()->begin(); b.isValid(); b = b.next())
        if (isCheckbox(b))
            blocks.append(b);

    int index = blocks.indexOf(block);

    if (index < 0)
        return -1;

    QList<Checkbox> lines = sourceCheckboxes(innerText);

    // The document and the source usually have the same checkboxes in the
    // same order.
    if (lines.size() == blocks.size())
        return lines.at(index).position;

    // Otherwise the source has something that only looks like a checkbox (an
    // indented code block, "[x]" without a space after the bracket), so the
    // line is found by its text.
    int occurrence = 0;

    for (int i = 0; i < index; i++)
        if (blocks.at(i).text() == block.text())
            occurrence++;

    for (const Checkbox& line : lines)
    {
        if (line.text != block.text().trimmed())
            continue;

        if (occurrence == 0)
            return line.position;

        occurrence--;
    }

    return -1;
}

// Only the single character of the source text is changed, so the rest of the
// note keeps its original markdown.
void StickerEdit::toggleCheckbox(QTextBlock block)
{
    int position = checkboxPosition(block);

    if (position < 0 || position >= innerText.size())
        return;

    QChar state = innerText.at(position);

    innerText[position] = state == ' ' ? QChar('x') : QChar(' ');

    QTextBlockFormat format = block.blockFormat();
    format.setMarker(state == ' '
        ? QTextBlockFormat::MarkerType::Checked
        : QTextBlockFormat::MarkerType::Unchecked);

    QTextCursor cursor(block);
    cursor.setBlockFormat(format);

    if (parent())
    {
        Sticker* sticker = (Sticker*)parent();

        sticker->notifyTextChanged();
    }
}

void StickerEdit::mousePressEvent(QMouseEvent* e)
{
    linkPressed = false;

    if (e->button() == Qt::LeftButton)
    {
        // The link is followed on release, the press is only swallowed here so
        // the editor does not take the focus.
        if (!anchorAt(e->position().toPoint()).isEmpty())
        {
            linkPressed = true;
            e->accept();
            return;
        }

        QTextBlock block = markerBlockAt(e->position());

        if (isCheckbox(block))
        {
            checkboxPressed = true;
            toggleCheckbox(block);
            e->accept();
            return;
        }
    }

    // The focus policy has no ClickFocus, so the editor is focused here and
    // focusInEvent switches the document back to the source text.
    if (!hasFocus())
        setFocus(Qt::MouseFocusReason);

    QTextEdit::mousePressEvent(e);
}

void StickerEdit::mouseMoveEvent(QMouseEvent* e)
{
    QTextEdit::mouseMoveEvent(e);

    if (isFocused)
        return;

    // The links are opened by the editor itself, and the text control does not
    // know about checkboxes, so the cursor shape is chosen here.
    QPointF pos = e->position();

    bool clickable = !anchorAt(pos.toPoint()).isEmpty()
        || isCheckbox(markerBlockAt(pos));

    viewport()->setCursor(clickable ? Qt::PointingHandCursor : Qt::IBeamCursor);
}

void StickerEdit::mouseReleaseEvent(QMouseEvent* e)
{
    if (linkPressed && e->button() == Qt::LeftButton)
    {
        linkPressed = false;

        QString link = anchorAt(e->position().toPoint());

        if (!link.isEmpty())
            QDesktopServices::openUrl(QUrl(link));

        e->accept();
        return;
    }

    // The text control toggles a checkbox on release, so the release of a
    // marker click is dropped as well or the marker is switched twice.
    if (checkboxPressed && e->button() == Qt::LeftButton)
    {
        checkboxPressed = false;
        e->accept();
        return;
    }

    QTextEdit::mouseReleaseEvent(e);
}

void StickerEdit::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton
        && (isCheckbox(markerBlockAt(e->position()))
            || !anchorAt(e->position().toPoint()).isEmpty()))
    {
        e->accept();
        return;
    }

    QTextEdit::mouseDoubleClickEvent(e);
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
        setPlainText(innerText);
    else
        setMarkdown(innerText);
}