#include "appsettings.h"

AppSettings::AppSettings(QObject *parent) : QObject(parent)
{
    Settings = new QSettings("Oleg Derevenetz", "MagicPhotos");
}

AppSettings::~AppSettings()
{
    delete Settings;
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
