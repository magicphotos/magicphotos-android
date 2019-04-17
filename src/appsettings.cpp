#include "appsettings.h"

AppSettings::AppSettings(QObject *parent) : QObject(parent)
{
    DefaultBrushSize = 0;
    Settings         = std::make_shared<QSettings>("Oleg Derevenetz", "MagicPhotos");
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
