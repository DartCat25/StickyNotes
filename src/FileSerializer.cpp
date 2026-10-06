#include "FileSerializer.h"
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <sstream>

const std::string dataNames[] = {
    "name",
    "position",
    "size",
    "rotation",
    "color",
    "stack"
};

enum StickerDataName {
    SDN_NAME,
    SDN_POSITION,
    SDN_SIZE,
    SDN_ROTATION,
    SDN_COLOR,
    SDN_STACK,
    SDN_UNKNOWN
};

StickerDataName getName(std::string name)
{
    for (int i = 0; i < SDN_UNKNOWN-1; ++i)
    {
        if (name == dataNames[i])
            return (StickerDataName)i;
    }
    return SDN_UNKNOWN;
}

int getNextInt(std::string& value, size_t& pos)
{
    int result = INT32_MAX;
    
    size_t i = pos;
    bool foundDigit = false;
    size_t digitPos = 0;

    for (; i < value.size(); ++i)
    {
        if (!foundDigit && (isdigit(value[i]) || value[i] == '-'))
        {
            foundDigit = true;
            digitPos = i;
        }
        else if (foundDigit && !isdigit(value[i]))
        {
            break;
        }
    }

    if (foundDigit)
        result = std::stoi(value.substr(digitPos, i - digitPos));

    pos = i;
    return result;
}

float getNextFloat(std::string& value, size_t& pos)
{
    int result = INT32_MAX;
    
    size_t i = pos;
    bool foundDigit = false;
    bool foundDot = false;
    size_t digitPos = 0;
    

    for (; i < value.size(); ++i)
    {
        if (!foundDigit && (isdigit(value[i]) || value[i] == '-'))
        {
            foundDigit = true;
            digitPos = i;
        }
        else if (foundDigit && !isdigit(value[i]) && value[i] != '.')
        {
            if (!foundDot && value[i] == '.')
                foundDot = true;
            else
                break;
        }
    }

    if (foundDigit)
        result = std::stof(value.substr(digitPos, i - digitPos));

    pos = i;
    return result;
}

/// Error if id = -1
///
/// Error message is in name
StickerData FileSerializer::deserialize(std::string text)
{
    StickerData data = {};

    size_t headStart = text.find("+++\n");
    if (headStart == -1)
    {
        data.id = -1;
        data.name = "No head";
        return data;
    }

    size_t headEnd = text.find("+++\n", headStart + 5);
    if (headEnd == -1)
    {
        data.id = -1;
        data.name = "No head end";
        return data;
    }

    size_t cursor = headStart + 5;
    size_t nextCursor = text.find(':', cursor);
    while (nextCursor == -1 || nextCursor > headEnd)
    {
        StickerDataName dataName = getName(text.substr(cursor, nextCursor-cursor));
        cursor = nextCursor+1;
        nextCursor = text.find('\n', cursor);
        std::string dataValue = text.substr(cursor, nextCursor-cursor);

        switch (dataName)
        {
        case SDN_NAME:
        {
            data.name = dataValue;
        }
        break;
        case SDN_POSITION:
        {
            size_t cursor = 0;
            data.x = getNextInt(dataValue, cursor);
            data.y = getNextInt(dataValue, cursor);
        }
        break;
        case SDN_SIZE:
        {
            size_t cursor = 0;
            data.w = getNextInt(dataValue, cursor);
            data.h = getNextInt(dataValue, cursor);
        }
        break;
        case SDN_ROTATION:
        {
            size_t cursor = 0;
            data.rotation = getNextFloat(dataValue, cursor);
        }
        break;
        case SDN_COLOR:
        {
            data.color = QColor::fromString(dataValue);
        }
        break;
        case SDN_STACK:
        {
            size_t cursor = 0;
            data.x = getNextInt(dataValue, cursor);
        }
        break;
        case SDN_UNKNOWN: default:
        break;
        }

        cursor = nextCursor + 1;
        nextCursor = text.find(':');
    }

    return data;
}

std::string FileSerializer::serialize(StickerData* data)
{
    std::stringstream strStr = std::stringstream();

    strStr
    //Header
    << "+++\n"
    << "name: " << data->name << "\n"
    << "position: " << data->x << " " << data->y << "\n"
    << "size: " << data->w << " " << data->h << "\n"
    << "rotation: " << data->rotation << "\n"
    << "color: " << data->color.name().toStdString() << "\n"
    << "stack: " << data->stackPos << "\n"
    << "+++\n"
    //Body
    << data->text;

    return strStr.str();
}