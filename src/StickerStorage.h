#include "structs.h"
#include <vector>

class StickerStorage {
public:
    StickerStorage();
    ~StickerStorage();

    uint32_t createSticker(StickerData data);
    bool removeSticker(uint32_t id);
    
private:
    StickerData* getSticker(uint32_t id);

    bool load(std::string path);
    bool save(std::string path);

    uint32_t lastId;
    std::vector<StickerData>stickers;
};