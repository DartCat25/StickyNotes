#include "structs.h"

namespace FileSerializer
{
    StickerData deserialize(std::string text);
    std::string serialize(StickerData* data);
};