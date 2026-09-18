#include "StickerWrapper.h"
#include <qbitmap.h>
#include <qgraphicsview.h>
#include <qguiapplication_platform.h>
#include <qmainwindow.h>
#include <qnamespace.h>
#include <QGraphicsProxyWidget>
#include <qpainter.h>
#include <qpoint.h>
#include <qpolygon.h>
#include <QWindow>
#include <qtransform.h>

#ifdef Q_OS_LINUX
#include <QGuiApplication>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif

#define WIDTH 412
#define HEIGHT 412

#define MAX_ANGLE 30.0
#define FORCE 0.1
#define FADE 0.94

// // Constructor / Destructor

StickerWrapper::StickerWrapper(QWidget* parent)
: QGraphicsView(parent)
{
    dragging = false;
    angle = 0;
    
    setAttribute(Qt::WA_DeleteOnClose); //Enforce deletion
    setAttribute(Qt::WA_TranslucentBackground);
        #ifdef Q_OS_LINUX
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    #else
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    #endif
    setMouseTracking(true);
    resize(WIDTH, HEIGHT);
    
    scene = new QGraphicsScene(this); 
    scene->setSceneRect(0, 0, WIDTH, HEIGHT);
    
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    
    setScene(scene);
    
    sticker = new Sticker(this);
    proxy = scene->addWidget(sticker);
    sticker->move(0, 0);

    proxy->setTransformOriginPoint(sticker->rect().center());
    proxy->setRotation(angle);

    proxy->setPos(WIDTH/2 - sticker->width()/2, HEIGHT/2 - sticker->height()/2);
    updateMask();
    show();
    
    #ifdef Q_OS_LINUX
    setupNative();
    #endif
}

#ifdef Q_OS_LINUX
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

StickerWrapper::~StickerWrapper()
{
}

// // External Events

void StickerWrapper::closeWindow()
{
    close();
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
}

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

// // Slot Events

void StickerWrapper::mouseMoveEvent(QMouseEvent *event)
{
    
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
        
        move(curPos.toPoint());
        rotate(newAngle);
        lastMousePos = current;
        event->accept();
        return;
    }

    QGraphicsView::mouseMoveEvent(event);
}