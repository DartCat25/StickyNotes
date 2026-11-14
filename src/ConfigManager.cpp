#include "ConfigManager.h"
#include <toml.hpp>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>
#include <qdebug.h>
#include <algorithm>

#define CONFIG_FILE "config.toml"
#define CONFIG_RESOURCE ":/assets/default_config.toml"

// // Constructor / Destructor

ConfigManager::ConfigManager()
{
    resetDefaults();
}

ConfigManager& ConfigManager::getInstance()
{
    static ConfigManager instance;
    return instance;
}

// // Getters

const Config& ConfigManager::getConfig() const
{
    return config;
}

const QString& ConfigManager::getPath() const
{
    return path;
}

// // Loading

bool ConfigManager::load()
{
    resetDefaults();
    path = defaultPath();

    if (!createDefaultFile())
        return false;

    return readFile();
}

QString ConfigManager::defaultPath()
{
    QString base = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);

    if (base.isEmpty())
        base = QCoreApplication::applicationDirPath();

    return base + "/" + CONFIG_FILE;
}

void ConfigManager::resetDefaults()
{
    config.sticker.color = QColor("#fef5c1");
    config.sticker.width = 300;
    config.sticker.height = 300;
    config.sticker.rotationEnabled = true;

    config.text.family = "";
    config.text.pointSize = 12;
    config.text.weight = 400;

    config.cloner.enabled = true;
    config.cloner.x = 0;
    config.cloner.y = 0;
    config.cloner.positionSet = false;
    config.cloner.anchorAtGrab = true;

    config.physics.fade = 0.94;
    config.physics.force = 0.1;
    config.physics.maxAngle = 30.0;
}

bool ConfigManager::createDefaultFile()
{
    if (QFile::exists(path))
        return true;

    QFileInfo info(path);

    if (!QDir().mkpath(info.absolutePath()))
    {
        qDebug() << "Config: cannot create directory" << info.absolutePath();
        return false;
    }

    QFile resource(CONFIG_RESOURCE);

    if (!resource.open(QIODevice::ReadOnly))
    {
        qDebug() << "Config: cannot open default config" << resource.errorString();
        return false;
    }

    QSaveFile file(path);

    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Config: cannot create" << path << file.errorString();
        return false;
    }

    if (file.write(resource.readAll()) < 0)
    {
        qDebug() << "Config: cannot write" << path << file.errorString();
        return false;
    }

    if (!file.commit())
    {
        qDebug() << "Config: cannot save" << path << file.errorString();
        return false;
    }

    return true;
}

bool ConfigManager::readFile()
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Config: cannot open" << path << file.errorString();
        return false;
    }

    std::string content = file.readAll().toStdString();
    std::string name = path.toStdString();

    toml::table table;

    try
    {
        table = toml::parse(content, name);
    }
    catch (const toml::parse_error& error)
    {
        qDebug() << "Config: parse error in" << path << ":"
                 << error.description().data();
        return false;
    }

    config.sticker.color = QColor(QString::fromStdString(
        table["sticker"]["color"].value_or("#fef5c1")));

    if (!config.sticker.color.isValid())
        config.sticker.color = QColor("#fef5c1");

    config.sticker.width = std::clamp(
        table["sticker"]["width"].value_or(300), 1, 10000);
    config.sticker.height = std::clamp(
        table["sticker"]["height"].value_or(300), 1, 10000);
    config.sticker.rotationEnabled =
        table["sticker"]["rotation_enabled"].value_or(true);

    config.text.family = QString::fromStdString(
        table["text"]["family"].value_or(""));
    config.text.pointSize = std::clamp(
        table["text"]["point_size"].value_or(12), 1, 200);
    config.text.weight = std::clamp(
        table["text"]["weight"].value_or(400), 100, 900);

    config.cloner.enabled = table["cloner"]["enabled"].value_or(true);
    config.cloner.anchorAtGrab = table["cloner"]["anchor_at_grab"].value_or(true);

    auto x = table["cloner"]["x"].value<int>();
    auto y = table["cloner"]["y"].value<int>();

    config.cloner.positionSet = x.has_value() && y.has_value();
    config.cloner.x = x.value_or(0);
    config.cloner.y = y.value_or(0);

    config.physics.fade = (float)std::clamp(
        table["physics"]["fade"].value_or(0.94), 0.0, 1.0);
    config.physics.force = (float)std::clamp(
        table["physics"]["force"].value_or(0.1), 0.0, 10.0);
    config.physics.maxAngle = (float)std::clamp(
        table["physics"]["max_angle"].value_or(30.0), 0.0, 180.0);

    return true;
}
