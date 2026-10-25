#include "StickerManager.h"
#include <QApplication>
#include <QScreen>

StickerManager::StickerManager()
{
    shuttingDown = false;
    storage = new StickerStorage();
    cloner = nullptr;

    for (const StickerData& data : storage->getStickers())
    {
        StickerWrapper* sticker = new StickerWrapper(data.id, &data);
        connectSticker(sticker);
    }

    createCloner(defaultClonerPos());
}

StickerManager::~StickerManager()
{
    shuttingDown = true;
    delete storage;
}

void StickerManager::onTransformChanged(
        int32_t x, int32_t y,
        uint32_t w, uint32_t h,
        float rotation)
{
    StickerWrapper* sticker = qobject_cast<StickerWrapper*>(sender());

    if (!sticker)
        return;

    if (sticker == cloner)
    {
        materializeCloner(sticker);
        return;
    }

    storage->setStickerTransform(
        sticker->getId(),
        x, y,
        w, h,
        rotation);
}

void StickerManager::onTextChanged(QString text)
{
    StickerWrapper* sticker = qobject_cast<StickerWrapper*>(sender());

    if (!sticker || sticker == cloner)
        return;

    storage->setStickerText(sticker->getId(), text.toStdString());
}

void StickerManager::onColorChanged(QColor color)
{
    StickerWrapper* sticker = qobject_cast<StickerWrapper*>(sender());

    if (!sticker || sticker == cloner)
        return;

    storage->setStickerColor(sticker->getId(), color);
}

void StickerManager::onStackPosChanged(StackPos pos)
{
    StickerWrapper* sticker = qobject_cast<StickerWrapper*>(sender());

    if (!sticker || sticker == cloner)
        return;

    storage->setStickerStackPos(sticker->getId(), pos);
}

void StickerManager::onCloseRequested()
{
    StickerWrapper* sticker = qobject_cast<StickerWrapper*>(sender());

    if (!sticker || shuttingDown)
        return;

    if (sticker == cloner)
    {
        shuttingDown = true;
        qApp->quit();
        return;
    }

    storage->removeSticker(sticker->getId());
}

void StickerManager::connectSticker(StickerWrapper* sticker)
{
    connect(sticker, &StickerWrapper::transformChanged,
        this, &StickerManager::onTransformChanged);

    connect(sticker, &StickerWrapper::textChanged,
        this, &StickerManager::onTextChanged);

    connect(sticker, &StickerWrapper::colorChanged,
        this, &StickerManager::onColorChanged);

    connect(sticker, &StickerWrapper::stackPosChanged,
        this, &StickerManager::onStackPosChanged);

    connect(sticker, &StickerWrapper::closeRequested,
        this, &StickerManager::onCloseRequested);
}

void StickerManager::createCloner(QPoint pos)
{
    cloner = new StickerWrapper(UINT32_MAX);
    connectSticker(cloner);

    #ifdef Q_OS_LINUX
    cloner->moveRaw(pos);
    #else
    cloner->move(pos);
    #endif

    cloner->raise();
}

void StickerManager::materializeCloner(StickerWrapper* sticker)
{
    StickerData data = sticker->getData();

    uint32_t id = storage->createSticker(data);

    sticker->setId(id);
    cloner = nullptr;

    createCloner(defaultClonerPos());
}

QPoint StickerManager::defaultClonerPos()
{
    QSize screenSize = QApplication::primaryScreen()->availableSize();

    // TODO: Take the position from the config singleton.
    return QPoint(
        screenSize.width() - 400,
        screenSize.height() - 100);
}
