#include "StickerStorage.h"
#include "StickerWrapper.h"

StickerStorage::StickerStorage()
{
    lastId = 0;
}

StickerStorage::~StickerStorage()
{
}

StickerWrapper* StickerStorage::createSticker(bool isCloner)
{
    StickerWrapper* newSticker = new StickerWrapper(lastId, isCloner);
    return newSticker;
}

bool StickerStorage::removeSticker(uint32_t id)
{
}

bool StickerStorage::loadFile(std::string path)
{
    return false;
}

bool StickerStorage::saveFile(std::string path)
{
    return false;
}