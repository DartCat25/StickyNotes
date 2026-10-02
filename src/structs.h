#pragma once
#include <cstdint>
#include <qcolor.h>
#include <string>

enum StackPos {
    SP_TOP,
    SP_FREE,
    SP_BOTTOM
};

struct StickerData {
    uint32_t id;
    std::string name;
    std::string text;
    uint32_t x, y;
    uint32_t w, h;
    float rotation;
    QColor color;
    StackPos stackPos;
};