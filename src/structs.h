#pragma once
#include <cstdint>
#include <qcolor.h>
#include <string>

struct StickerData {
    uint32_t id;
    std::string name;
    std::string text;
    uint32_t x, y;
    uint32_t w, h;
    float rotation;
    uint32_t color;
};