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

bool AppSettings::showModeChangeSuggestion() const
{
    if (Settings->contains("ShowModeChangeSuggestion")) {
        return Settings->value("ShowModeChangeSuggestion").toBool();
    } else {
        return true;
    }
}

void AppSettings::setShowModeChangeSuggestion(const bool &show)
{
    Settings->setValue("ShowModeChangeSuggestion", show);
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
