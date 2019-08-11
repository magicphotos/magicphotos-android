#include "appsettings.h"

AppSettings::AppSettings(QObject *parent) : QObject(parent)
{
    DefaultBrushSize = 0;
    Settings         = std::make_shared<QSettings>("Oleg Derevenetz", "MagicPhotos");
}

AppSettings &AppSettings::GetInstance()
{
    static AppSettings instance;

    return instance;
}

bool AppSettings::disableAds() const
{
    if (Settings->contains("DisableAds")) {
        return Settings->value("DisableAds").toBool();
    } else if (Settings->contains("IsFullVersion")) {
        return Settings->value("IsFullVersion").toBool();
    } else {
        return false;
    }
}

void AppSettings::setDisableAds(bool disable)
{
    Settings->setValue("DisableAds", disable);
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

QString AppSettings::adMobConsent() const
{
    if (Settings->contains("AdMobConsent")) {
        return Settings->value("AdMobConsent").toString();
    } else {
        return "";
    }
}

void AppSettings::setAdMobConsent(QString &consent)
{
    Settings->setValue("AdMobConsent", consent);
}
