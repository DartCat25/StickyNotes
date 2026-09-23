#include "Sticker.h"
#include "StickerWrapper.h"
#include <qaction.h>
#include <qapplication.h>
#include <qboxlayout.h>
#include <qcoreevent.h>
#include <qdebug.h>
#include <qevent.h>
#include <qframe.h>
#include <qlogging.h>
#include <qmenu.h>
#include <qnamespace.h>
#include <qpushbutton.h>
#include <QVBoxLayout>
#include <qsizepolicy.h>
#include <qtextedit.h>
#include <qwidget.h>
#include <QMenu>
#include <QColorDialog>

#define WIDTH 300
#define HEIGHT 300

#define HEADER_HEIGHT 40
#define CLOSE_BTN_SIZE 20

#define TEXT_MARGIN 10

#define SIZE_MARGIN 20

// // Constructor / destructor

Sticker::Sticker(StickerWrapper* parent) : QFrame()
{
    wrapper = parent;
    scalingSide = 0;
    
    setObjectName("Sticker");
    setBaseSize(WIDTH, HEIGHT);
    resize(WIDTH, HEIGHT);

    QVBoxLayout* vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(0, 0, 0, 0);

    // // Header
    
    header = new QFrame();
    header->setFixedHeight(HEADER_HEIGHT);
    header->setSizePolicy(QSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Fixed));
    header->setMouseTracking(true);
    header->installEventFilter(this);

    vbox->addWidget(header, 0);
    
    QHBoxLayout* hbox = new QHBoxLayout(header);

    QWidget* spacer = new QWidget();

    hbox->addWidget(spacer, 1);

    QPushButton* closeButton = new QPushButton("X");
    closeButton->setStyleSheet("border: none;");
    closeButton->setFixedSize(CLOSE_BTN_SIZE, CLOSE_BTN_SIZE);
    connect(
        closeButton,
        &QPushButton::clicked,
        this,
        &Sticker::closeButton_clicked);

    hbox->addWidget(closeButton, 0);

    // // Body
    QSizePolicy expand = QSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Expanding);

    textEdit = new QTextEdit();
    textEdit->setSizePolicy(expand);
    textEdit->setStyleSheet("border: none; font-size:12pt;");
    textEdit->setMinimumSize(0, 0);
    textEdit->installEventFilter(this);

    vbox->addWidget(textEdit, 1);

    setBackgroundColor("#fef5c1");
}

Sticker::~Sticker()
{

}

// // External events

bool Sticker::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == header &&
        (
            event->type() == QEvent::MouseButtonPress
            || event->type() == QEvent::MouseButtonRelease
        ))
    {
        QMouseEvent* mouseEvent = (QMouseEvent*)event;

        if (event->type() == QEvent::MouseButtonPress)
            header_pressed(mouseEvent);
        else
            header_released(mouseEvent);
    }

    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyPress = (QKeyEvent*) event;
        if (keyPress->key() == Qt::Key_Escape)
        {
            setFocus();
            event->accept();
            return true;
        }
    }

    if (watched == textEdit)
    {
        if (event->type() == QEvent::FocusIn)
        {
            textEdit->setPlainText(textEdit->toMarkdown());
        }
        else if (event->type() == QEvent::FocusOut)
        {
            textEdit->setMarkdown(textEdit->toPlainText());
        }
    }

    return QFrame::eventFilter(watched, event);
}

// // Internal events

void Sticker::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        lastMousePos = event->position();
        QPointF clickPos = event->position();
        QSize sz = size();


        if (clickPos.x() < SIZE_MARGIN)
            scalingSide |= ScalingSide::LEFT;
        else if (clickPos.x() > sz.width() - SIZE_MARGIN)
            scalingSide |= ScalingSide::RIGHT;

        if (clickPos.y() < SIZE_MARGIN)
            scalingSide |= ScalingSide::TOP;
        else if (clickPos.y() > sz.width() - SIZE_MARGIN)
            scalingSide |= ScalingSide::BOTTOM;

        if (scalingSide)
            return;
    }

    QFrame::mousePressEvent(event);
}

void Sticker::mouseMoveEvent(QMouseEvent* event)
{
    if (!scalingSide)
    {
        QFrame::mouseMoveEvent(event);
        return;    
    }

    QPointF current = event->position();

    QPointF vec = current - lastMousePos;

    QPointF curPos = pos();
    QSize curSize = size();

    if (scalingSide & ScalingSide::TOP)
    {
        move(curPos.x(), curPos.y() + vec.y());
        resize(curSize.width(), curSize.height() - vec.y());
    }
    if (scalingSide & ScalingSide::BOTTOM)
        resize(curSize.width(), curSize.height() + vec.y());

    curPos = pos();
    curSize = size();

    if (scalingSide & ScalingSide::LEFT)
    {
        move(curPos.x() + vec.x(), curPos.y());
        resize(curSize.width() - vec.x(), curSize.height());
    }
    if (scalingSide & ScalingSide::RIGHT)
        resize(curSize.width() + vec.x(), curSize.height());

    lastMousePos = current;
    wrapper->updateMask();
}

void Sticker::mouseReleaseEvent(QMouseEvent* event)
{
    scalingSide = 0;
    QFrame::mouseReleaseEvent(event);
}

void Sticker::closeButton_clicked()
{
    wrapper->closeWindow();
}

void Sticker::header_pressed(QMouseEvent* event)
{
    wrapper->startDragging(event->globalPosition());
}

void Sticker::header_released(QMouseEvent* event)
{
    wrapper->endDragging();
}

//Context menu
void Sticker::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(nullptr);

    
    // // Stacking
    QMenu* stacking = menu.addMenu("Stacking");

    QAction *alOnTop    = stacking->addAction("Set always on Top");
    QAction *free       = stacking->addAction("Set free stacking");
    QAction *alOnBottom = stacking->addAction("Set always on Bottom");
    
    alOnTop->setCheckable(true);
    free->setCheckable(true);
    alOnBottom->setCheckable(true);

    StackPos pos = wrapper->getStackPos();

    alOnTop->setChecked(pos == SP_TOP);
    free->setChecked(pos == SP_FREE);
    alOnBottom->setChecked(pos == SP_BOTTOM);


    // // Setting color
    QAction* setColor = menu.addAction("Set Color");

    // // Start
    QAction *selectedAction = menu.exec(event->globalPos());

    if (selectedAction == alOnTop)
        wrapper->setStackPos(SP_TOP);
    if (selectedAction == free)
        wrapper->setStackPos(SP_FREE);
    if (selectedAction == alOnBottom)
        wrapper->setStackPos(SP_BOTTOM);
    if (selectedAction == setColor)
    {
        QColor color = QColorDialog::getColor("#fef5c1");
        setBackgroundColor(color);
    }
}

// // Getters and setters
void Sticker::setBackgroundColor(QColor color)
{
    int h = 0, s = 0, l = 0;
    color.getHsl(&h, &s, &l);

    //Calculate lumninance
    float r = 0, g = 0, b = 0;
    color.getRgbF(&r, &g, &b);
    r = std::pow(r, 2.2);
    g = std::pow(g, 2.2);
    b = std::pow(b, 2.2);
    float L = 0.2126 * r + 0.7152 * g + 0.0722 * b;

    QColor headerColor = QColor::fromHsl(
        h,
        std::max(int(s * 0.9), 0),
        std::max(int(l * 0.9), 0));

    QColor borderColor = QColor::fromHsl(
        h,
        std::min(int(s * 1.2f), 255),
        std::max(int(l * 0.8f), 0));

    QString penColor = L > 0.179 ? "black" : "white";

    setStyleSheet("QWidget {background: " + color.name()
    + "; color: " + penColor
    + ";} QWidget#Sticker {border: 2px solid " + borderColor.name() + ";}"
    );
    header->setStyleSheet("background: " + headerColor.name() + "; color: " + penColor + ";");

    backgroundColor = color;
}

QColor Sticker::getBackgroundColor()
{
    return backgroundColor;
}

void Sticker::setText(QString text)
{
    textEdit->setMarkdown(text);
}

QString Sticker::getText()
{
    return textEdit->toPlainText();
}