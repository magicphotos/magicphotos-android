#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QSettings>

class AppSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool    disableAds       READ disableAds       WRITE setDisableAds)
    Q_PROPERTY(int     defaultBrushSize READ defaultBrushSize WRITE setDefaultBrushSize)
    Q_PROPERTY(int     brushSize        READ brushSize        WRITE setBrushSize)
    Q_PROPERTY(qreal   brushHardness    READ brushHardness    WRITE setBrushHardness)
    Q_PROPERTY(QString adMobConsent     READ adMobConsent     WRITE setAdMobConsent)

private:
    explicit AppSettings(QObject *parent = nullptr);
    ~AppSettings() noexcept override = default;

public:
    AppSettings(const AppSettings &) = delete;
    AppSettings(AppSettings &&) noexcept = delete;

    AppSettings &operator=(const AppSettings &) = delete;
    AppSettings &operator=(AppSettings &&) noexcept = delete;

    static AppSettings &GetInstance();

    bool disableAds() const;
    void setDisableAds(bool disable);

    int defaultBrushSize() const;
    void setDefaultBrushSize(int size);

    int brushSize() const;
    void setBrushSize(int size);

    qreal brushHardness() const;
    void setBrushHardness(qreal hardness);

    QString adMobConsent() const;
    void setAdMobConsent(const QString &consent);

private:
    int       DefaultBrushSize;
    QSettings Settings {QStringLiteral("Oleg Derevenetz"), QStringLiteral("MagicPhotos")};
};

#endif // APPSETTINGS_H
