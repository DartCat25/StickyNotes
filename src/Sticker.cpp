#include "Sticker.h"
#include "StickerWrapper.h"
#include <qnamespace.h>
#include <qpushbutton.h>

Sticker::Sticker(QWidget* parent)
    : QWidget(parent)
{
    QPushButton* testButton = new QPushButton("close", this);
    connect(
        testButton,
        &QPushButton::clicked,
        this,
        &Sticker::testButton_clicked);
}

Sticker::~Sticker()
{

}

void Sticker::testButton_clicked()
{
    if (parent())
        ((StickerWrapper*)parentWidget())->closeWindow();
}