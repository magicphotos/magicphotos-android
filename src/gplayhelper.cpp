#include <QtAndroidExtras/QtAndroid>

#include "gplayhelper.h"

GPlayHelper::GPlayHelper(QObject *parent) :
    QObject(parent)
{
}

GPlayHelper &GPlayHelper::GetInstance()
{
    static GPlayHelper instance;

    return instance;
}

void GPlayHelper::requestReview() const
{
    QtAndroid::androidActivity().callMethod<void>("requestReview");
}
