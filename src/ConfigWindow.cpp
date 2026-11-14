#include "ConfigWindow.h"
#include "ui_ConfigWindow.h"
#include <QApplication>
#include <QColorDialog>
#include <QDesktopServices>
#include <QFont>
#include <QIcon>
#include <QMessageBox>
#include <QPixmap>
#include <QProcess>
#include <QUrl>

ConfigWindow::ConfigWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::ConfigWindow)
{
    ui->setupUi(this);

    setConfig(ConfigManager::getInstance().getConfig());
}

ConfigWindow::~ConfigWindow()
{
    delete ui;
}

// // Slots

void ConfigWindow::on_colorButton_clicked()
{
    QColor selected = QColorDialog::getColor(color, this, "Sticker Color");

    if (!selected.isValid())
        return;

    color = selected;
    updateColorButton();
}

void ConfigWindow::on_customPositionCheck_toggled(bool checked)
{
    ui->xSpin->setEnabled(checked);
    ui->ySpin->setEnabled(checked);
}

void ConfigWindow::on_resetButton_clicked()
{
    setConfig(ConfigManager::getDefaultConfig());
}

void ConfigWindow::on_openFileButton_clicked()
{
    QDesktopServices::openUrl(
        QUrl::fromLocalFile(ConfigManager::getInstance().getPath()));
}

void ConfigWindow::on_cancelButton_clicked()
{
    close();
}

void ConfigWindow::on_applyButton_clicked()
{
    if (!ConfigManager::getInstance().save(getConfig()))
    {
        QMessageBox::warning(this, "StickyNotes",
            "Cannot save the config file.");
        return;
    }

    if (!QProcess::startDetached(QCoreApplication::applicationFilePath(),
        QCoreApplication::arguments().mid(1)))
    {
        QMessageBox::warning(this, "StickyNotes",
            "Cannot restart the application.");
        return;
    }

    qApp->quit();
}

// // Configuration

void ConfigWindow::setConfig(const Config& config)
{
    ui->rotationCheck->setChecked(config.sticker.rotationEnabled);
    ui->anchorCheck->setChecked(config.sticker.anchorAtGrab);

    QFont font = QApplication::font();

    if (!config.text.family.isEmpty())
        font.setFamily(config.text.family);

    ui->fontCombo->setCurrentFont(font);
    ui->sizeSpin->setValue(config.text.pointSize);
    ui->weightSpin->setValue(config.text.weight);

    ui->enabledCheck->setChecked(config.cloner.enabled);
    ui->customPositionCheck->setChecked(config.cloner.positionSet);
    ui->xSpin->setValue(config.cloner.x);
    ui->ySpin->setValue(config.cloner.y);

    color = config.cloner.color;
    updateColorButton();

    ui->widthSpin->setValue((int)config.cloner.width);
    ui->heightSpin->setValue((int)config.cloner.height);

    ui->fadeSpin->setValue(config.physics.fade);
    ui->forceSpin->setValue(config.physics.force);
    ui->maxAngleSpin->setValue(config.physics.maxAngle);
}

Config ConfigWindow::getConfig() const
{
    Config config;

    config.sticker.rotationEnabled = ui->rotationCheck->isChecked();
    config.sticker.anchorAtGrab = ui->anchorCheck->isChecked();

    config.text.family = ui->fontCombo->currentFont().family();
    config.text.pointSize = ui->sizeSpin->value();
    config.text.weight = ui->weightSpin->value();

    config.cloner.enabled = ui->enabledCheck->isChecked();
    config.cloner.positionSet = ui->customPositionCheck->isChecked();
    config.cloner.x = ui->xSpin->value();
    config.cloner.y = ui->ySpin->value();
    config.cloner.color = color;
    config.cloner.width = (uint32_t)ui->widthSpin->value();
    config.cloner.height = (uint32_t)ui->heightSpin->value();

    config.physics.fade = (float)ui->fadeSpin->value();
    config.physics.force = (float)ui->forceSpin->value();
    config.physics.maxAngle = (float)ui->maxAngleSpin->value();

    return config;
}

void ConfigWindow::updateColorButton()
{
    QPixmap pixmap(16, 16);
    pixmap.fill(color);

    ui->colorButton->setIcon(QIcon(pixmap));
    ui->colorButton->setText(color.name());
}
