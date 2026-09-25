
#include "StickerWrapper.h"
#include <string>

class StickerStorage {
public:
    StickerStorage();
    ~StickerStorage();

    StickerWrapper* createSticker(bool isCloner);
    bool removeSticker(uint32_t id);
    
    bool loadFile(std::string path);
    bool saveFile(std::string path);
private:
    uint32_t lastId;
};