#include "StickerManager.h"
#include "ConfigManager.h"
#include <QApplication>
#include <QScreen>
#include <QAction>
#include <QDesktopServices>
#include <QIcon>
#include <QUrl>

StickerManager::StickerManager()
{
    shuttingDown = false;
    storage = new StickerStorage();
    cloner = nullptr;

    //Config tray
    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(QIcon(":/assets/icon.png"), this);

    if (ConfigManager::getInstance().getConfig().cloner.enabled)
        trayIcon->setToolTip("StickyNotes");
    else
        trayIcon->setToolTip("StickyNotes - double click to create a sticker");

    menu = new QMenu();

    QAction* openFolderAction = menu->addAction("Open Stickers Folder");
    connect(openFolderAction, &QAction::triggered,
        this, &StickerManager::onOpenStickersFolder);

    QAction* quitAction = menu->addAction("Quit");
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    connect(trayIcon, &QSystemTrayIcon::activated, this, &StickerManager::onTrayIconActivated);

    trayIcon->setContextMenu(menu);
    trayIcon->show();

    //Load stickers from storage
    for (const StickerData& data : storage->getStickers())
    {
        StickerWrapper* sticker = new StickerWrapper(data.id, &data);
        connectSticker(sticker);
    }

    if (ConfigManager::getInstance().getConfig().cloner.enabled)
        createCloner(defaultClonerPos());
}

StickerManager::~StickerManager()
{
    shuttingDown = true;
    delete storage;
    delete menu;
}

// // Signals

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

void StickerManager::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger)
    {
        if (!cloner)
            return;

        cloner->activateWindow();
        return;
    }

    if (reason == QSystemTrayIcon::DoubleClick && !cloner)
        createStickerAt(screenCenter());
}

void StickerManager::onOpenStickersFolder()
{
    QDesktopServices::openUrl(
        QUrl::fromLocalFile(QString::fromStdString(storage->getPath())));
}

// // Sticker Management

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
    persistSticker(sticker);

    cloner = nullptr;

    createCloner(defaultClonerPos());
}

void StickerManager::persistSticker(StickerWrapper* sticker)
{
    StickerData data = sticker->getData();

    uint32_t id = storage->createSticker(data);

    sticker->setId(id);
}

void StickerManager::createStickerAt(QPoint pos)
{
    StickerWrapper* sticker = new StickerWrapper(UINT32_MAX, SP_FREE);
    connectSticker(sticker);

    sticker->move(pos - sticker->rect().center());

    persistSticker(sticker);
}

QPoint StickerManager::defaultClonerPos()
{
    const Config& config = ConfigManager::getInstance().getConfig();

    if (config.cloner.positionSet)
    {
        QPoint pos(config.cloner.x, config.cloner.y);

        for (QScreen* screen : QApplication::screens())
        {
            if (screen->availableGeometry().contains(pos))
                return pos;
        }
    }

    QSize screenSize = QApplication::primaryScreen()->availableSize();

    return QPoint(
        screenSize.width() - 400,
        screenSize.height() - 100);
}

QPoint StickerManager::screenCenter()
{
    return QApplication::primaryScreen()->availableGeometry().center();
}
