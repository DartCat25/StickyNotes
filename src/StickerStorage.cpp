#include "StickerStorage.h"
#include "FileSerializer.h"
#include "structs.h"
#include <QStandardPaths>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <qdebug.h>
#include <sstream>
#include <string>

StickerStorage::StickerStorage(std::string path)
{
    setPath(path);
}

StickerStorage::~StickerStorage()
{
}

std::string StickerStorage::defaultPath()
{
    QString base = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    if (base.isEmpty())
        return "stickers";

    return (base + "/StickyNotes/stickers").toStdString();
}

void StickerStorage::setPath(std::string path)
{
    if (path.empty())
        path = defaultPath();

    this->path = path;
    trashPath = path + "/trash";
    lastId = 0;
    lastNameId = 0;
    stickers.clear();

    initDirs();
    loadDir(path);
}

uint32_t StickerStorage::createSticker(StickerData data)
{
    data.id = lastId++;
    data.name = generateName();

    saveFile(getFileName(&data), &data);
    stickers.push_back(data);

    return data.id;
}

bool StickerStorage::removeSticker(uint32_t id)
{
    StickerData* data = getSticker(id);

    if (!data)
        return false;

    std::error_code error;
    std::filesystem::rename(
        getFileName(data),
        trashPath + "/" + data->name + ".stk",
        error);

    if (error)
    {
        qDebug() << error.message().c_str();
        return false;
    }

    for (size_t i = 0; i < stickers.size(); ++i)
    {
        if (stickers[i].id != id)
            continue;

        stickers.erase(stickers.begin() + i);
        break;
    }

    return true;
}

void StickerStorage::setStickerTransform(
        uint32_t id,
        int32_t x, int32_t y,
        uint32_t w, uint32_t h,
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

void StickerStorage::setStickerText(uint32_t id, std::string text)
{
    StickerData* data = getSticker(id);

    if (!data)
        return;

    data->text = text;

    save(data);
}

void StickerStorage::setStickerColor(uint32_t id, QColor color)
{
    StickerData* data = getSticker(id);

    if (!data)
        return;

    data->color = color;

    save(data);
}

void StickerStorage::setStickerStackPos(uint32_t id, StackPos pos)
{
    StickerData* data = getSticker(id);

    if (!data)
        return;

    data->stackPos = pos;

    save(data);
}

const std::vector<StickerData>& StickerStorage::getStickers() const
{
    return stickers;
}

StickerData* StickerStorage::getSticker(uint32_t id)
{
    if (stickers.empty())
        return nullptr;

    size_t min = 0;
    size_t max = stickers.size();

    while (min < max)
    {
        size_t element = (min + max) / 2;

        if (stickers[element].id == id)
            return &stickers[element];

        if (stickers[element].id < id)
            min = element + 1;
        else
            max = element;
    }

    return nullptr;
}

void StickerStorage::save(StickerData* sticker)
{
    saveFile(getFileName(sticker), sticker);
}

StickerData StickerStorage::loadFile(std::string path)
{
    std::ifstream stream(path);

    if (!stream.is_open())
    {
        StickerData data = {};
        data.id = -1U;
        data.name = "No file exists: " + path;
        return data;
    }

    std::stringstream text;
    text << stream.rdbuf();
    stream.close();

    return FileSerializer::deserialize(text.str());
}

bool StickerStorage::saveFile(std::string path, StickerData* data)
{
    std::ofstream stream(path);

    if (!stream.is_open())
        return false;

    stream << FileSerializer::serialize(data);

    stream.close();

    return true;
}

std::string StickerStorage::getFileName(StickerData* data)
{
    return path + "/" + data->name + ".stk";
}

std::string StickerStorage::generateName()
{
    return "sticker" + std::to_string(lastNameId++);
}

void StickerStorage::loadDir(std::string path)
{
    std::error_code error;
    std::filesystem::directory_iterator iterator(path, error);

    if (error)
    {
        qDebug() << error.message().c_str();
        return;
    }

    for (const std::filesystem::directory_entry& entry : iterator)
    {
        if (!entry.is_regular_file())
            continue;

        if (entry.path().extension() != ".stk")
            continue;

        StickerData data = loadFile(entry.path().string());

        if (data.id == -1U)
        {
            qDebug() << data.name.c_str();
            continue;
        }

        data.id = lastId++;
        data.name = entry.path().stem().string();

        updateLastNameId(data.name);
        stickers.push_back(data);
    }
}

void StickerStorage::updateLastNameId(std::string name)
{
    std::string prefix = "sticker";

    if (name.size() <= prefix.size())
        return;

    if (name.compare(0, prefix.size(), prefix) != 0)
        return;

    std::string number = name.substr(prefix.size());

    for (char symbol : number)
    {
        if (!isdigit(symbol))
            return;
    }

    uint32_t value = std::stoul(number);

    if (value >= lastNameId)
        lastNameId = value + 1;
}

void StickerStorage::initDirs()
{
    std::error_code error;
    std::filesystem::create_directories(trashPath, error);

    if (error)
        qDebug() << error.message().c_str();
}
