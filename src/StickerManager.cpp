#include "StickerManager.h"

StickerManager::StickerManager()
{
    storage = new StickerStorage("");
}

StickerManager::~StickerManager()
{
    delete storage;
}