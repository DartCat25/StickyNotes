#pragma once

#include <QByteArray>
#include <QColor>
#include <QString>
#include <cstdint>

struct StickerConfig {
    bool rotationEnabled;
    bool anchorAtGrab;
};

struct TextConfig {
    QString family;
    int pointSize;
    int weight;
};

struct ClonerConfig {
    bool enabled;
    int x;
    int y;
    bool positionSet;
    QColor color;
    uint32_t width;
    uint32_t height;
};

struct PhysicsConfig {
    float fade;
    float force;
    float maxAngle;
};

struct Config {
    StickerConfig sticker;
    TextConfig text;
    ClonerConfig cloner;
    PhysicsConfig physics;
};

class ConfigManager {
public:
    static ConfigManager& getInstance();

    bool load();
    bool save(const Config& newConfig);

    const Config& getConfig() const;
    const QString& getPath() const;

    static Config getDefaultConfig();

    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

private:
    ConfigManager();

    static QString defaultPath();
    static void fillDefaults(Config& config);
    static bool parseConfig(
        const QByteArray& content,
        const QString& name,
        Config& config);
    static QString serialize(const Config& config);
    static QString formatFloat(float value);

    bool createDefaultFile();
    bool readFile();

    Config config;
    QString path;
};
