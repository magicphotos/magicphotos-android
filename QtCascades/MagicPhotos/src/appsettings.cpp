#include "appsettings.h"

AppSettings::AppSettings(QObject *parent) : QObject(parent)
{
    Settings = new QSettings("Oleg Derevenetz", "MagicPhotos");
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
