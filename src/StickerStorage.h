#pragma once

#include "structs.h"
#include <cstdint>
#include <string>
#include <vector>

class StickerStorage {
public:
    StickerStorage(std::string path = defaultPath());
    ~StickerStorage();

    static std::string defaultPath();
    void setPath(std::string path);

    uint32_t createSticker(StickerData data);
    bool removeSticker(uint32_t id);

    void setStickerTransform(
        uint32_t id,
        int32_t x, int32_t y,
        uint32_t w, uint32_t h,
        float rotation);

    void setStickerText(uint32_t id, std::string text);
    void setStickerColor(uint32_t id, QColor color);
    void setStickerStackPos(uint32_t id, StackPos pos);

    const std::vector<StickerData>& getStickers() const;
private:
    StickerData* getSticker(uint32_t id);

    void save(StickerData* sticker);

    StickerData loadFile(std::string path);
    bool saveFile(std::string path, StickerData* data);

    std::string getFileName(StickerData* data);
    std::string generateName();

    void loadDir(std::string path);
    void updateLastNameId(std::string name);
    void initDirs();

    uint32_t lastId;
    uint32_t lastNameId;
    std::vector<StickerData> stickers;
    std::string path;
    std::string trashPath;
};
