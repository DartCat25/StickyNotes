#include "StickerStorage.h"
#include "FileSerializer.h"
#include "structs.h"
#include <qdebug.h>
#include <qlogging.h>
#include <sstream>
#include <string>
#include <cstdio>
#include <filesystem>
#include <fstream>

StickerStorage::StickerStorage(std::string path)
{
    lastId = 0;
    stickers = std::vector<StickerData>();
    this->path = path;
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

void StickerStorage::changeStickerTransform(
        uint32_t id,
        int32_t x, int32_t y,
        uint32_t h, uint32_t w,
        float rotation)
{
    StickerData* data = getSticker(id);

    if (!data)
        return;

    data->x = x; data->y = y;
    data->w = w; data->h = h;
    data->rotation = rotation;

    save(data);
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

        if (stickers[element].id > id)
            max = element;
        else
            min = element;
        element = (min + max) / 2;
    }

    return {};
}

std::vector<StickerData> StickerStorage::loadDir(std::string path)
{
    StickerData data;
    for (const std::filesystem::directory_entry& entry
        : std::filesystem::recursive_directory_iterator(path))
    {
        if (!entry.is_regular_file())
            continue;

        data = loadFile(entry.path());

        if (data.id == -1U) //Show error
        {
            qDebug() << data.name;
            continue;
        }
            
        stickers.push_back(data);
    }

    return stickers;
}

void StickerStorage::save(StickerData* sticker)
{

}

StickerData StickerStorage::loadFile(std::string path)
{
    std::ifstream stream(path);
    if (stream.bad())
        return {.id = -1U, .name = "No file exists: " + path};

    std::stringstream text;
    text << stream.rdbuf();
    stream.close();

    StickerData data = FileSerializer::deserialize(text.str());
    if (data.id == -1U) //Catch error messages
    {
        return data;
    }

    data.id = lastId++;

    return data;
}

bool StickerStorage::saveFile(std::string path, StickerData* data)
{
    std::ofstream stream(path);

    if (stream.bad())
        return false;

    stream << FileSerializer::serialize(data);

    stream.close();

    return true;
}

std::string StickerStorage::getFileName(StickerData* data)
{
    return path + "/" + data->name + ".stk";
}

std::string StickerStorage::generateName(StickerData* data)
{
    return "sticker" + std::to_string(data->id);
}
