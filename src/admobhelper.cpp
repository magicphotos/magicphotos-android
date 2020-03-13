#include <QtAndroidExtras/QtAndroid>
#include <QtAndroidExtras/QAndroidJniObject>

#include "admobhelper.h"

const QString AdMobHelper::ADMOB_BANNERVIEW_UNIT_ID  (QStringLiteral("ca-app-pub-2455088855015693/1811713388"));
const QString AdMobHelper::ADMOB_INTERSTITIAL_UNIT_ID(QStringLiteral("ca-app-pub-2455088855015693/6662028461"));

AdMobHelper::AdMobHelper(QObject *parent) :
    QObject         (parent),
    Initialized     (false),
    BannerViewHeight(0)
{
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

void AdMobHelper::setPersonalization(bool personalized) const
{
    jboolean j_personalized = personalized ? JNI_TRUE : JNI_FALSE;

    QtAndroid::androidActivity().callMethod<void>("setAdsPersonalization", "(Z)V", j_personalized);
}

void AdMobHelper::showBannerView() const
{
    if (Initialized) {
        QAndroidJniObject j_unit_id = QAndroidJniObject::fromString(ADMOB_BANNERVIEW_UNIT_ID);

        QtAndroid::androidActivity().callMethod<void>("showBannerView", "(Ljava/lang/String;)V", j_unit_id.object<jstring>());
    }
}

void AdMobHelper::hideBannerView() const
{
    if (Initialized) {
        QtAndroid::androidActivity().callMethod<void>("hideBannerView");
    }
}

void AdMobHelper::showInterstitial() const
{
    if (Initialized) {
        QtAndroid::androidActivity().callMethod<void>("showInterstitial");
    }
}

void AdMobHelper::setBannerViewHeight(int height)
{
    if (BannerViewHeight != height) {
        BannerViewHeight = height;

        emit bannerViewHeightChanged(BannerViewHeight);
    }
}
