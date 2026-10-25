#include "StickerWrapper.h"
#include <qbitmap.h>
#include <qcoreevent.h>
#include <qdebug.h>
#include <qevent.h>
#include <qgraphicsview.h>
#include <qguiapplication_platform.h>
#include <qlogging.h>
#include <qmainwindow.h>
#include <qnamespace.h>
#include <QGraphicsProxyWidget>
#include <qpainter.h>
#include <qpoint.h>
#include <qpolygon.h>
#include <QWindow>
#include <qtransform.h>
#include <qwidget.h>
#include <xcb/xproto.h>

#ifdef Q_OS_LINUX
#include <QGuiApplication>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <xcb/xcb.h>
#endif

#define WIDTH 412
#define HEIGHT 412

#define MAX_ANGLE 30.0
#define FORCE 0.1
#define FADE 0.94

// // Constructor / Destructor

StickerWrapper::StickerWrapper(uint32_t id, StackPos pos, QWidget* parent)
: QGraphicsView(parent)
{
    dragging = false;
    angle = 0;
    stackPos = SP_FREE;
    this->id = id;
    
    setAttribute(Qt::WA_DeleteOnClose); //Enforce deletion
    setAttribute(Qt::WA_TranslucentBackground);
    #ifdef Q_OS_LINUX
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    #else
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    #endif
    setMouseTracking(true);
    resize(WIDTH, HEIGHT);
    setStyleSheet("background: transparent;");

    scene = new QGraphicsScene(this); 
    scene->setSceneRect(0, 0, WIDTH, HEIGHT);
    
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    
    setScene(scene);
    
    sticker = new Sticker(this);
    proxy = scene->addWidget(sticker);
    sticker->move(0, 0);

    sticker->installEventFilter(this);

    proxy->setTransformOriginPoint(sticker->rect().center());
    proxy->setRotation(angle);

    proxy->setPos(WIDTH/2 - sticker->width()/2, HEIGHT/2 - sticker->height()/2);

    setStackPos(pos, false);
    initWindow();
}

StickerWrapper::StickerWrapper(uint32_t id, const StickerData* data, QWidget* parent)
: StickerWrapper(id, data ? data->stackPos : SP_FREE, parent)
{
    if (data)
        applyData(data);
}

StickerWrapper::~StickerWrapper()
{
}

void StickerWrapper::applyData(const StickerData* data)
{
    if (!data)
        return;

    setStackPos(data->stackPos);
    setSize(data->w, data->h);
    setAngle(data->rotation);
    setBackgroundColor(data->color);
    setText(QString::fromStdString(data->text));

    #ifdef Q_OS_LINUX
    moveRaw(QPoint(data->x, data->y));
    #else
    move(data->x, data->y);
    #endif
}

StickerData StickerWrapper::getData()
{
    StickerData data = {};

    data.id = id;
    data.x = pos().x();
    data.y = pos().y();
    data.w = sticker->width();
    data.h = sticker->height();
    data.rotation = angle;
    data.color = getBackgroundColor();
    data.text = getText().toStdString();
    data.stackPos = stackPos;

    return data;
}

void StickerWrapper::initWindow()
{
    show();
    updateMask();
    #ifdef Q_OS_LINUX
    setupNative();
    #endif
}

void StickerWrapper::recreateWindow()
{
    hide();
    destroy(true, true);
    createWinId();
    initWindow();
}

// // External Events

void StickerWrapper::closeWindow()
{
    close();
}

void StickerWrapper::closeEvent(QCloseEvent* event)
{
    emit closeRequested();
    QGraphicsView::closeEvent(event);
}

void StickerWrapper::startDragging(QPointF start)
{
    lastMousePos = start;
    QTransform rotInvert = QTransform();
    rotInvert.rotate(-angle);

    QPointF center = QPointF(WIDTH/2.0, HEIGHT/2.0);
    pivot = rotInvert.map(start - pos() - center) + center;
    dragging = true;
}

void StickerWrapper::endDragging()
{
    dragging = false;
    notifyTransformChanged();
}

// // Getters and setters

void StickerWrapper::setStackPos(StackPos pos, bool reInit)
{
    if (pos == stackPos)
        return;

    switch (pos)
    {
    case SP_TOP:
        setWindowFlag(Qt::WindowStaysOnTopHint, true);
        setWindowFlag(Qt::WindowStaysOnBottomHint, false);
        break;
    case SP_FREE:
        setWindowFlag(Qt::WindowStaysOnTopHint, false);
        setWindowFlag(Qt::WindowStaysOnBottomHint, false);
        break;
    case SP_BOTTOM:
        setWindowFlag(Qt::WindowStaysOnTopHint, false);
        setWindowFlag(Qt::WindowStaysOnBottomHint, true);
        break;
    }
    stackPos = pos;

    if (reInit)
        recreateWindow();

    emit stackPosChanged(pos);
}
StackPos StickerWrapper::getStackPos() {return stackPos;}

void StickerWrapper::setAngle(float angle) { rotate(angle); }
float StickerWrapper::getAngle() { return angle; }

void StickerWrapper::setSize(uint32_t w, uint32_t h)
{
    sticker->resize(w, h);
    proxy->setTransformOriginPoint(sticker->rect().center());
    updateMask();
}

//Proxy functions
void StickerWrapper::setBackgroundColor(QColor color)
{
    sticker->setBackgroundColor(color);
    emit colorChanged(color);
}
QColor StickerWrapper::getBackgroundColor() {return sticker->getBackgroundColor();}

void StickerWrapper::setText(QString text) {return sticker->setText(text);}
QString StickerWrapper::getText() {return sticker->getText();}

// // Inner funcs

void StickerWrapper::rotate(float angle)
{
    proxy->setRotation(angle);
    this->angle = angle;
    updateMask();
}

void StickerWrapper::updateMask()
{
    QPolygonF scenePath = proxy->mapToScene(sticker->rect());   
    QRegion maskRegion(scenePath.toPolygon());
    
    setMask(maskRegion);
}

void StickerWrapper::notifyTransformChanged()
{
    emit transformChanged(
        pos().x(), pos().y(),
        sticker->width(), sticker->height(),
        angle);
}

void StickerWrapper::notifyTextChanged()
{
    emit textChanged(getText());
}

#ifdef Q_OS_LINUX
//Raw move to bypass screen limits
void StickerWrapper::moveRaw(QPoint point)
{
    auto *x11App =
        qApp->nativeInterface<QNativeInterface::QX11Application>();

    if (!x11App)
        return;

    Display *display = x11App->display();
    Window window = winId();

    Atom atom = XInternAtom(
        display,
        "_NET_MOVERESIZE_WINDOW",
        False
    );

    if (atom == None)
        return;

    XEvent event{};
    event.xclient.type = ClientMessage;
    event.xclient.window = window;
    event.xclient.message_type = atom;
    event.xclient.format = 32;

    // StaticGravity + X + Y + source=application
    event.xclient.data.l[0] =
        StaticGravity |
        (1L << 8) |
        (1L << 9) |
        (1L << 12);

    event.xclient.data.l[1] = point.x();
    event.xclient.data.l[2] = point.y();

    XSendEvent(
        display,
        DefaultRootWindow(display),
        False,
        SubstructureRedirectMask | SubstructureNotifyMask,
        &event
    );

    XFlush(display);
}

//Native X11 way to remove sticker from taskbar
void StickerWrapper::setupNative()
{
    QNativeInterface::QX11Application *x11App = qApp->nativeInterface<QNativeInterface::QX11Application>();
    if (x11App) {
        Display *display = x11App->display();
        Window wId = this->winId();

        Atom wmState = XInternAtom(display, "_NET_WM_STATE", False);
        Atom skipTaskbar = XInternAtom(display, "_NET_WM_STATE_SKIP_TASKBAR", False);

        if (wmState != None && skipTaskbar != None) {
            XEvent xev;
            memset(&xev, 0, sizeof(xev));
            xev.type = ClientMessage;
            xev.xclient.window = wId;
            xev.xclient.message_type = wmState;
            xev.xclient.format = 32;
            xev.xclient.data.l[0] = 1; // 1 = _NET_WM_STATE_ADD (add flag)
            xev.xclient.data.l[1] = skipTaskbar;
            xev.xclient.data.l[2] = 0;
            xev.xclient.data.l[3] = 1;

            XSendEvent(display, DefaultRootWindow(display), False,
                    SubstructureRedirectMask | SubstructureNotifyMask, &xev);
            XFlush(display);
        }
    }
}
#endif

// // Slot Events

void StickerWrapper::mouseMoveEvent(QMouseEvent *event)
{
    // qDebug() << window()->geometry() << window()->frameGeometry();
    if ((event->buttons() & Qt::LeftButton) && dragging)
    {
        QPointF current = event->globalPosition();
        QPointF curPos = pos();
        QPointF vec = current - lastMousePos;

        double newAngle = std::min(std::max(angle * FADE + vec.x() * FORCE, -MAX_ANGLE), MAX_ANGLE);
        
        QPointF center = proxy->mapToScene(proxy->rect().center());
        
        QTransform rotateTransf = QTransform();
        rotateTransf.rotate(newAngle);
        
        curPos = current - rotateTransf.map(pivot - center) - center;
        
        #ifdef  Q_OS_LINUX
        moveRaw(curPos.toPoint());
        #else
        move(curPos.toPoint());
        #endif
        rotate(newAngle);
        lastMousePos = current;
        event->accept();
        return;
    }

    QGraphicsView::mouseMoveEvent(event);
}