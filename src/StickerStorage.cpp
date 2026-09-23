#include "StickerStorage.h"
#include "StickerWrapper.h"

StickerStorage::StickerStorage()
{
    first = last = nullptr;
    lastId = 0;
}

StickerStorage::~StickerStorage()
{
    if (first)
        delete first;
    first = last = nullptr;
}

StickerWrapper* StickerStorage::createSticker(bool isCloner)
{
    StickerWrapper* newSticker = new StickerWrapper(lastId, isCloner);
    if (!first)
    {
        first = last = newSticker;
        return newSticker;
    }

    last->setNext(newSticker);
    last = newSticker;

    return newSticker;
}

bool StickerStorage::removeSticker(uint32_t id)
{
    //No elements
    if (!first)
        return false;

    //First element
    if (first->getId() == id)
    {
        if (last == first) last = nullptr;
        first = first->getNext();
        return true;
    }

    StickerWrapper* itLast = first;
    StickerWrapper* it = first->getNext();

    while (it)
    {
        if (it->getId() == id)
        {
            if (it == last)
                last = itLast;
            itLast->setNext(it->getNext());
            it->setNext(nullptr);
            return true;
        }
    }

    //Not found
    return false;
}

bool StickerStorage::loadFile(std::string path)
{
    return false;
}

bool StickerStorage::saveFile(std::string path)
{
    return false;
}