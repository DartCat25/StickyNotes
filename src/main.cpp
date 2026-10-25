#include "StickerManager.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    #ifdef Q_OS_LINUX
    setenv("QT_QPA_PLATFORM", "xcb", 1);
    #endif
    QApplication a(argc, argv);

    StickerManager manager;

    return a.exec();
}
