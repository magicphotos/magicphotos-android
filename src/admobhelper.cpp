#include <QtAndroidExtras/QtAndroid>
#include <QtAndroidExtras/QAndroidJniObject>

#include "admobhelper.h"

const QString AdMobHelper::ADMOB_APP_ID              ("ca-app-pub-2455088855015693~2942020662");
const QString AdMobHelper::ADMOB_BANNERVIEW_UNIT_ID  ("ca-app-pub-2455088855015693/8624758039");
const QString AdMobHelper::ADMOB_INTERSTITIAL_UNIT_ID("ca-app-pub-2455088855015693/4390833707");

AdMobHelper::AdMobHelper(QObject *parent) : QObject(parent)
{
    BannerViewHeight = 0;

    QAndroidJniObject j_app_id               = QAndroidJniObject::fromString(ADMOB_APP_ID);
    QAndroidJniObject j_interstitial_unit_id = QAndroidJniObject::fromString(ADMOB_INTERSTITIAL_UNIT_ID);

    QtAndroid::androidActivity().callMethod<void>("initAds", "(Ljava/lang/String;Ljava/lang/String;)V", j_app_id.object<jstring>(),
                                                                                                        j_interstitial_unit_id.object<jstring>());
}

int AdMobHelper::bannerViewHeight() const
{
    return BannerViewHeight;
}

void AdMobHelper::showBannerView()
{
    QAndroidJniObject j_unit_id = QAndroidJniObject::fromString(ADMOB_BANNERVIEW_UNIT_ID);

    QtAndroid::androidActivity().callMethod<void>("showBannerView", "(Ljava/lang/String;)V", j_unit_id.object<jstring>());
}

void AdMobHelper::hideBannerView()
{
    QtAndroid::androidActivity().callMethod<void>("hideBannerView");
}

void AdMobHelper::showInterstitial()
{
    QtAndroid::androidActivity().callMethod<void>("showInterstitial");
}

void AdMobHelper::setBannerViewHeight(int height)
{
    BannerViewHeight = height;

    emit bannerViewHeightChanged(BannerViewHeight);
}
