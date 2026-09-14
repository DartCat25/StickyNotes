#pragma once

#include <qtmetamacros.h>
#include <qwidget.h>

class Sticker : public QWidget
{
public:
    Sticker(QWidget* parent = nullptr);
    ~Sticker();

private:
    void testButton_clicked();
};