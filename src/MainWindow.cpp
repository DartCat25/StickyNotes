#include "MainWindow.h"
#include "StickerWrapper.h"
#include "ui_MainWindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->stackCombo->addItems({"Always on Top", "Free", "Always On Bottom"});
    stackPos = StackPos::SP_TOP;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_createStickerBtn_clicked()
{
    StickerWrapper* stWrapper = new StickerWrapper(false, stackPos);
    stWrapper->show();
}

void MainWindow::on_stackCombo_currentIndexChanged()
{
    switch (ui->stackCombo->currentIndex())
    {
        case 0: stackPos = StackPos::SP_TOP; break;
        case 1: stackPos = StackPos::SP_FREE; break;
        case 2: stackPos = StackPos::SP_BOTTOM; break;
    }
}