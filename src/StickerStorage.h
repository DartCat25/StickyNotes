#include "structs.h"
#include <vector>

class StickerStorage {
public:
    StickerStorage(std::string path);
    ~StickerStorage();

    uint32_t createSticker(StickerData data);
    bool removeSticker(uint32_t id);

    void changeStickerTransform(
        uint32_t id,
        int32_t x, int32_t y,
        uint32_t h, uint32_t w,
        float rotation);

    std::vector<StickerData> loadDir(std::string path);
private:
    StickerData* getSticker(uint32_t id);

    void save(StickerData* sticker);

    StickerData loadFile(std::string path);
    bool saveFile(std::string path, StickerData* data);

    std::string getFileName(StickerData* data);
    std::string generateName(StickerData* data);

    uint32_t lastId;
    std::vector<StickerData> stickers;
    std::string path;
};