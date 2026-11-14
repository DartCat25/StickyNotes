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
    config.sticker.rotationEnabled = true;
    config.sticker.anchorAtGrab = true;

    config.text.family = "";
    config.text.pointSize = 12;
    config.text.weight = 400;

    config.cloner.enabled = true;
    config.cloner.x = 0;
    config.cloner.y = 0;
    config.cloner.positionSet = false;
    config.cloner.color = QColor("#fef5c1");
    config.cloner.width = 300;
    config.cloner.height = 300;

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

    config.sticker.rotationEnabled =
        table["sticker"]["ROTATION_ENABLED"].value_or(true);
    config.sticker.anchorAtGrab =
        table["sticker"]["ANCHOR_AT_GRAB"].value_or(true);

    config.text.family = QString::fromStdString(
        table["text"]["FAMILY"].value_or(""));
    config.text.pointSize = std::clamp(
        table["text"]["POINT_SIZE"].value_or(12), 1, 200);
    config.text.weight = std::clamp(
        table["text"]["WEIGHT"].value_or(400), 100, 900);

    config.cloner.enabled = table["cloner"]["ENABLED"].value_or(true);

    config.cloner.color = QColor(QString::fromStdString(
        table["cloner"]["COLOR"].value_or("#fef5c1")));

    if (!config.cloner.color.isValid())
        config.cloner.color = QColor("#fef5c1");

    config.cloner.width = std::clamp(
        table["cloner"]["WIDTH"].value_or(300), 1, 10000);
    config.cloner.height = std::clamp(
        table["cloner"]["HEIGHT"].value_or(300), 1, 10000);

    auto x = table["cloner"]["X"].value<int>();
    auto y = table["cloner"]["Y"].value<int>();

    config.cloner.positionSet = x.has_value() && y.has_value();
    config.cloner.x = x.value_or(0);
    config.cloner.y = y.value_or(0);

    config.physics.fade = (float)std::clamp(
        table["physics"]["FADE"].value_or(0.94), 0.0, 1.0);
    config.physics.force = (float)std::clamp(
        table["physics"]["FORCE"].value_or(0.1), 0.0, 10.0);
    config.physics.maxAngle = (float)std::clamp(
        table["physics"]["MAX_ANGLE"].value_or(30.0), 0.0, 180.0);

    return true;
}
