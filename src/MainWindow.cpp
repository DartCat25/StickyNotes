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

void MainWindow::on_createStickerBtn_clicked()
{
    StickerWrapper* stWrapper = new StickerWrapper();
    stWrapper->show();
}
