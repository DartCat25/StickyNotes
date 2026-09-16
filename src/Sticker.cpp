#include "Sticker.h"
#include "StickerWrapper.h"
#include <qboxlayout.h>
#include <qdebug.h>
#include <qevent.h>
#include <qframe.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qpushbutton.h>
#include <QVBoxLayout>
#include <qsizepolicy.h>
#include <qwidget.h>

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
    
    setStyleSheet("background: #fef5c1;");

    setBaseSize(WIDTH, HEIGHT);
    resize(WIDTH, HEIGHT);

    QVBoxLayout* vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(0, 0, 0, 0);

    // // Header
    
    header = new QFrame();
    header->setStyleSheet("background: #f7e9a2;");
    header->setFixedHeight(HEADER_HEIGHT);
    header->setSizePolicy(QSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Fixed));
    header->setMouseTracking(true);
    header->installEventFilter(this);

    vbox->addWidget(header, 0);
    
    QHBoxLayout* hbox = new QHBoxLayout(header);

    QLineEdit* nameEdit = new QLineEdit("Noname");
    nameEdit->setStyleSheet("border: none;");
    nameEdit->setCursor(Qt::ArrowCursor);

    hbox->addWidget(nameEdit, 1);

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

    QFrame* bodyFrame = new QFrame();
    bodyFrame->setSizePolicy(expand);
    bodyFrame->setContentsMargins(TEXT_MARGIN, TEXT_MARGIN, TEXT_MARGIN, TEXT_MARGIN);

    textEdit = new QTextEdit(bodyFrame);
    textEdit->setSizePolicy(expand);

    vbox->addWidget(bodyFrame, 1);
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


    if (scalingSide & ScalingSide::LEFT)
    {
        move(curPos.x() + vec.x(), curPos.y());
        resize(curSize.width() - vec.x(), curSize.height());
    }
    if (scalingSide & ScalingSide::RIGHT)
        resize(curSize.width() + vec.x(), curSize.height());

    lastMousePos = current;
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