#ifndef ADMOBHELPER_H
#define ADMOBHELPER_H

#include <QtCore/QObject>
#include <QtCore/QString>

class AdMobHelper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int bannerViewHeight READ bannerViewHeight NOTIFY bannerViewHeightChanged)

public:
    static const QString ADMOB_APP_ID,
                         ADMOB_BANNERVIEW_UNIT_ID,
                         ADMOB_INTERSTITIAL_UNIT_ID;

    explicit AdMobHelper(QObject *parent = 0);
    virtual ~AdMobHelper();

    int bannerViewHeight() const;

    Q_INVOKABLE void showBannerView();
    Q_INVOKABLE void hideBannerView();
    Q_INVOKABLE void showInterstitial();

signals:
    void bannerViewHeightChanged(int bannerViewHeight);

public slots:
    void setBannerViewHeight(int height);

private:
    int BannerViewHeight;
};

#endif // ADMOBHELPER_H
