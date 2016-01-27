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

int AppSettings::launchNumber() const
{
    if (Settings->contains("LaunchNumber")) {
        return Settings->value("LaunchNumber").toInt();
    } else {
        return 0;
    }
}

void AppSettings::setLaunchNumber(const int &number)
{
    Settings->setValue("LaunchNumber", number);
}

int AppSettings::lastFeedbackRequestLaunchNumber() const
{
    if (Settings->contains("LastFeedbackRequestLaunchNumber")) {
        return Settings->value("LastFeedbackRequestLaunchNumber").toInt();
    } else {
        return 0;
    }
}

void AppSettings::setLastFeedbackRequestLaunchNumber(const int &number)
{
    Settings->setValue("LastFeedbackRequestLaunchNumber", number);
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

bool AppSettings::requestFeedback() const
{
    if (Settings->contains("RequestFeedback")) {
        return Settings->value("RequestFeedback").toBool();
    } else {
        return true;
    }
}

void AppSettings::setRequestFeedback(const bool &request)
{
    Settings->setValue("RequestFeedback", request);
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

qreal AppSettings::imageResolutionLimit() const
{
    if (Settings->contains("ImageResolutionLimit")) {
        return Settings->value("ImageResolutionLimit").toDouble();
    } else {
        return 1.0;
    }
}

void AppSettings::setImageResolutionLimit(const qreal &limit)
{
    Settings->setValue("ImageResolutionLimit", limit);
}
