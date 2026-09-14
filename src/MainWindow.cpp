#include "MainWindow.h"
#include "StickerWrapper.h"
#include "ui_MainWindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Qt automatically connects this slot to 'myButton' because of the naming convention:
// on_<objectName>_<signalName>
void MainWindow::on_createStickerBtn_clicked()
{
    // QMessageBox::information(this, "Hello!", "You clicked the button!");
    StickerWrapper* stWrapper = new StickerWrapper();
    stWrapper->show();
}
