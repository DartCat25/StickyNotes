#include "MainWindow.h"
#include "StickerWrapper.h"
#include <QApplication>
#include <qapplication.h>

int main(int argc, char *argv[])
{
    #ifdef Q_OS_LINUX
    setenv("QT_QPA_PLATFORM", "xcb", 1);
    #endif
    QApplication a(argc, argv);
    // MainWindow w;
    // w.show();

    QSize screenSize = QApplication::primaryScreen()->availableSize();

    QPoint startPoint = QPoint( screenSize.width() - 400, screenSize.height() - 100);

    StickerWrapper* first = new StickerWrapper(0, true);
    #ifdef Q_OS_LINUX
    first->moveRaw(startPoint);
    #else
    first->move(startPoint);
    #endif
    return a.exec();
}
