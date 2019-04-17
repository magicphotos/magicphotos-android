#include "appsettings.h"

AppSettings::AppSettings(QObject *parent) : QObject(parent)
{
    DefaultBrushSize = 0;
    Settings         = new QSettings("Oleg Derevenetz", "MagicPhotos");
}

AppSettings::~AppSettings() noexcept
{
    delete Settings;
}

int AppSettings::defaultBrushSize() const
{
    return DefaultBrushSize;
}

void AppSettings::setDefaultBrushSize(int size)
{
    DefaultBrushSize = size;
}

int AppSettings::brushSize() const
{
    if (Settings->contains("BrushSize")) {
        return Settings->value("BrushSize").toInt();
    } else {
        return DefaultBrushSize;
    }
}

void AppSettings::setBrushSize(int size)
{
    Settings->setValue("BrushSize", size);
}

bool AppSettings::disableAds() const
{
#ifdef FULL_VERSION
    return true;
#else
    if (Settings->contains("DisableAds")) {
        return Settings->value("DisableAds").toBool();
    } else if (Settings->contains("IsFullVersion")) {
        return Settings->value("IsFullVersion").toBool();
    } else {
        return false;
    }
#endif
}

void AppSettings::setDisableAds(bool disable)
{
    Settings->setValue("DisableAds", disable);
}

qreal AppSettings::brushHardness() const
{
    if (Settings->contains("BrushHardness")) {
        return Settings->value("BrushHardness").toDouble();
    } else {
        return 0.75;
    }
}

void AppSettings::setBrushHardness(qreal hardness)
{
    Settings->setValue("BrushHardness", hardness);
}
