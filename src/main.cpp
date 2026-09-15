#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    setenv("QT_QPA_PLATFORM", "xcb", 1);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
