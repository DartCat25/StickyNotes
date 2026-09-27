#include "StickerStorage.h"
#include "structs.h"
#include <sstream>
#include <string>
#include <cstdio>

StickerStorage::StickerStorage()
{
    lastId = 0;
}

StickerStorage::~StickerStorage()
{
}

uint32_t StickerStorage::createSticker(StickerData data)
{
    data.id = lastId++;

    stickers.push_back(data);

    return data.id;
}

bool StickerStorage::removeSticker(uint32_t id)
{
    StickerData* data = getSticker(id);

    std::string path = data->name + ".stk";
    int err = std::remove(path.c_str());

    return true;
}

StickerData* StickerStorage::getSticker(uint32_t id)
{
    if (!stickers.size())
        return {};

    size_t min = 0;
    size_t max = stickers.size()-1;

    size_t element = max/2;
    
    uint32_t idMax = stickers[max].id;

    if (idMax == id)
        return &stickers[element];

    while (max != element && min != element)
    {
        if (stickers[element].id == id)
            return &stickers[element];

        if (stickers[element].id < id)
            max = element;
        else
            min = element;
        element = (min + max) / 2;
    }

    return {};
}

bool StickerStorage::load(std::string path)
{
    return false;
}

bool StickerStorage::save(std::string path)
{
    return false;
}

std::string StickerStorage::serialize(StickerData* data)
{

}

StickerData StickerStorage::deserialize(std::string text)
{

}