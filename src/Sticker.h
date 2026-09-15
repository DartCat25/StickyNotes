#pragma once

#include <qframe.h>
#include <qtmetamacros.h>
#include <qwidget.h>
#include <QLineEdit>

class Sticker : public QFrame
{
public:
    Sticker(QWidget* parent = nullptr);
    ~Sticker();

private:
    void closeButton_clicked();

    QLineEdit* nameEdit;
};