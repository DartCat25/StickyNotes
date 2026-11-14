#include "ConfigManager.h"
#include <toml.hpp>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTextStream>
#include <qdebug.h>
#include <algorithm>

#define CONFIG_FILE "config.toml"
#define CONFIG_RESOURCE ":/assets/default_config.toml"

// // Constructor / Destructor

ConfigManager::ConfigManager()
{
    fillDefaults(config);
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
    config = getDefaultConfig();
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

Config ConfigManager::getDefaultConfig()
{
    Config defaults;

    fillDefaults(defaults);

    QFile resource(CONFIG_RESOURCE);

    if (!resource.open(QIODevice::ReadOnly))
    {
        qDebug() << "Config: cannot open default config"
                 << resource.errorString();
        return defaults;
    }

    Config config;

    if (!parseConfig(resource.readAll(), CONFIG_RESOURCE, config))
        return defaults;

    return config;
}

void ConfigManager::fillDefaults(Config& config)
{
    config.sticker.rotationEnabled = true;
    config.sticker.anchorAtGrab = false;

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

    return parseConfig(file.readAll(), path, config);
}

bool ConfigManager::parseConfig(
    const QByteArray& content,
    const QString& name,
    Config& config)
{
    toml::table table;

    try
    {
        table = toml::parse(content.toStdString(), name.toStdString());
    }
    catch (const toml::parse_error& error)
    {
        qDebug() << "Config: parse error in" << name << ":"
                 << error.description().data();
        return false;
    }

    config.sticker.rotationEnabled =
        table["sticker"]["ROTATION_ENABLED"].value_or(true);
    config.sticker.anchorAtGrab =
        table["sticker"]["ANCHOR_AT_GRAB"].value_or(false);

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

// // Saving

bool ConfigManager::save(const Config& newConfig)
{
    QFileInfo info(path);

    if (!QDir().mkpath(info.absolutePath()))
    {
        qDebug() << "Config: cannot create directory" << info.absolutePath();
        return false;
    }

    QSaveFile file(path);

    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Config: cannot write" << path << file.errorString();
        return false;
    }

    QByteArray content = serialize(newConfig).toUtf8();

    if (file.write(content) != content.size())
    {
        qDebug() << "Config: cannot write" << path << file.errorString();
        return false;
    }

    if (!file.commit())
    {
        qDebug() << "Config: cannot save" << path << file.errorString();
        return false;
    }

    config = newConfig;

    return true;
}

QString ConfigManager::serialize(const Config& config)
{
    QString family = config.text.family;
    family.replace("\\", "\\\\");
    family.replace("\"", "\\\"");
    family.replace("\n", "\\n");

    QString position;

    if (config.cloner.positionSet)
    {
        position = QString("X = %1\nY = %2\n")
            .arg(config.cloner.x)
            .arg(config.cloner.y);
    }
    else
        position = "# X = 100\n# Y = 100\n";

    QString output;
    QTextStream stream(&output);

    stream << "# StickyNotes settings.\n";
    stream << "# Missing keys fall back to the defaults, unknown keys are ignored.\n";
    stream << "\n";
    stream << "[sticker]\n";
    stream << "# Tilt the sticker while it is dragged.\n";
    stream << "ROTATION_ENABLED = "
        << (config.sticker.rotationEnabled ? "true" : "false") << "\n";
    stream << "# Hinge the tilt at the grabbed point.\n";
    stream << "ANCHOR_AT_GRAB = "
        << (config.sticker.anchorAtGrab ? "true" : "false") << "\n";
    stream << "\n";
    stream << "[text]\n";
    stream << "# Empty family means the system font.\n";
    stream << "FAMILY = \"" << family << "\"\n";
    stream << "POINT_SIZE = " << config.text.pointSize << "\n";
    stream << "# 100..900, QFont::Weight.\n";
    stream << "WEIGHT = " << config.text.weight << "\n";
    stream << "\n";
    stream << "[cloner]\n";
    stream << "# Without the cloner a sticker is created by double clicking the tray icon.\n";
    stream << "ENABLED = "
        << (config.cloner.enabled ? "true" : "false") << "\n";
    stream << "# Position of the cloner window, defaults to the bottom right corner of the\n";
    stream << "# primary screen.\n";
    stream << position;
    stream << "# Defaults of a new sticker.\n";
    stream << "COLOR = \"" << config.cloner.color.name() << "\"\n";
    stream << "WIDTH = " << config.cloner.width << "\n";
    stream << "HEIGHT = " << config.cloner.height << "\n";
    stream << "\n";
    stream << "[physics]\n";
    stream << "# How fast the tilt decays, 0..1.\n";
    stream << "FADE = " << formatFloat(config.physics.fade) << "\n";
    stream << "# Tilt strength of horizontal mouse movement.\n";
    stream << "FORCE = " << formatFloat(config.physics.force) << "\n";
    stream << "# Tilt limit in degrees.\n";
    stream << "MAX_ANGLE = " << formatFloat(config.physics.maxAngle) << "\n";
    stream.flush();

    return output;
}

QString ConfigManager::formatFloat(float value)
{
    QString text = QString::number(value, 'f', 4);

    while (text.endsWith('0') && !text.endsWith(".0"))
        text.chop(1);

    return text;
}
