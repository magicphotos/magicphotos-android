#include <QtAndroidExtras/QtAndroid>
#include <QtAndroidExtras/QAndroidJniObject>

#include "admobhelper.h"

const QString AdMobHelper::ADMOB_BANNERVIEW_UNIT_ID  ("ca-app-pub-2455088855015693/8624758039");
const QString AdMobHelper::ADMOB_INTERSTITIAL_UNIT_ID("ca-app-pub-2455088855015693/4390833707");

AdMobHelper::AdMobHelper(QObject *parent) : QObject(parent)
{
    Initialized      = false;
    BannerViewHeight = 0;
}

AdMobHelper &AdMobHelper::GetInstance()
{
    static AdMobHelper instance;

    return instance;
}

int AdMobHelper::bannerViewHeight() const
{
    return BannerViewHeight;
}

void AdMobHelper::initAds()
{
    if (!Initialized) {
        QAndroidJniObject j_interstitial_unit_id = QAndroidJniObject::fromString(ADMOB_INTERSTITIAL_UNIT_ID);

        QtAndroid::androidActivity().callMethod<void>("initAds", "(Ljava/lang/String;)V", j_interstitial_unit_id.object<jstring>());

        Initialized = true;
    }
}

void AdMobHelper::setPersonalization(bool personalized)
{
    jboolean j_personalized = personalized ? JNI_TRUE : JNI_FALSE;

    QtAndroid::androidActivity().callMethod<void>("setAdsPersonalization", "(Z)V", j_personalized);
}

void AdMobHelper::showBannerView()
{
    if (Initialized) {
        QAndroidJniObject j_unit_id = QAndroidJniObject::fromString(ADMOB_BANNERVIEW_UNIT_ID);

        QtAndroid::androidActivity().callMethod<void>("showBannerView", "(Ljava/lang/String;)V", j_unit_id.object<jstring>());
    }
}

void AdMobHelper::hideBannerView()
{
    if (Initialized) {
        QtAndroid::androidActivity().callMethod<void>("hideBannerView");
    }
}

void AdMobHelper::showInterstitial()
{
    if (Initialized) {
        QtAndroid::androidActivity().callMethod<void>("showInterstitial");
    }
}

void AdMobHelper::setBannerViewHeight(int height)
{
    BannerViewHeight = height;

    emit bannerViewHeightChanged(BannerViewHeight);
}
