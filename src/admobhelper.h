#ifndef ADMOBHELPER_H
#define ADMOBHELPER_H

#include <QtCore/QObject>
#include <QtCore/QString>

class AdMobHelper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int bannerViewHeight READ bannerViewHeight NOTIFY bannerViewHeightChanged)

private:
    explicit AdMobHelper(QObject *parent = nullptr);
    ~AdMobHelper() noexcept override = default;

public:
    static const QString ADMOB_BANNERVIEW_UNIT_ID,
                         ADMOB_INTERSTITIAL_UNIT_ID;

    AdMobHelper(const AdMobHelper &) = delete;
    AdMobHelper(AdMobHelper &&) noexcept = delete;

    AdMobHelper &operator=(const AdMobHelper &) = delete;
    AdMobHelper &operator=(AdMobHelper &&) noexcept = delete;

    static AdMobHelper &GetInstance();

    int bannerViewHeight() const;

    Q_INVOKABLE void initAds();

    Q_INVOKABLE void setPersonalization(bool personalized) const;

    Q_INVOKABLE void showBannerView() const;
    Q_INVOKABLE void hideBannerView() const;

    Q_INVOKABLE void showInterstitial() const;

public slots:
    void setBannerViewHeight(int height);

signals:
    void bannerViewHeightChanged(int bannerViewHeight);

private:
    bool Initialized;
    int  BannerViewHeight;
};

#endif // ADMOBHELPER_H
