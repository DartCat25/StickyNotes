#pragma once

#include "ConfigManager.h"
#include <QColor>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class ConfigWindow; }
QT_END_NAMESPACE

class ConfigWindow : public QMainWindow
{
    Q_OBJECT

public:
    ConfigWindow(QWidget* parent = nullptr);
    ~ConfigWindow();

private slots:
    void on_colorButton_clicked();
    void on_customPositionCheck_toggled(bool checked);
    void on_resetButton_clicked();
    void on_openFileButton_clicked();
    void on_cancelButton_clicked();
    void on_applyButton_clicked();

private:
    void setConfig(const Config& config);
    Config getConfig() const;
    void updateColorButton();

    Ui::ConfigWindow* ui;

    QColor color;
};
