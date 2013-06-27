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

int AppSettings::lastRequestLaunchNumber() const
{
    if (Settings->contains("LastRequestLaunchNumber")) {
        return Settings->value("LastRequestLaunchNumber").toInt();
    } else {
        return 0;
    }
}

void AppSettings::setLastRequestLaunchNumber(const int &number)
{
    Settings->setValue("LastRequestLaunchNumber", number);
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
