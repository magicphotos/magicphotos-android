#include "appsettings.h"

AppSettings::AppSettings(QObject *parent) : QObject(parent)
{
    DefaultBrushSize = 0;
    Settings         = new QSettings("Oleg Derevenetz", "MagicPhotos");
}

AppSettings::~AppSettings()
{
    delete Settings;
}

int AppSettings::defaultBrushSize() const
{
    return DefaultBrushSize;
}

void AppSettings::setDefaultBrushSize(const int &size)
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

void AppSettings::setBrushSize(const int &size)
{
    Settings->setValue("BrushSize", size);
}

bool AppSettings::isFullVersion() const
{
#ifdef FULL_VERSION
    return true;
#else
    if (Settings->contains("IsFullVersion")) {
        return Settings->value("IsFullVersion").toBool();
    } else {
        return false;
    }
#endif
}

void AppSettings::setIsFullVersion(const bool &full)
{
    Settings->setValue("IsFullVersion", full);
}

qreal AppSettings::brushOpacity() const
{
    if (Settings->contains("BrushOpacity")) {
        return Settings->value("BrushOpacity").toDouble();
    } else {
        return 0.75;
    }
}

void AppSettings::setBrushOpacity(const qreal &opacity)
{
    Settings->setValue("BrushOpacity", opacity);
}
