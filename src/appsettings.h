#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <memory>

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QSettings>

class AppSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool  disableAds       READ disableAds       WRITE setDisableAds)
    Q_PROPERTY(int   defaultBrushSize READ defaultBrushSize WRITE setDefaultBrushSize)
    Q_PROPERTY(int   brushSize        READ brushSize        WRITE setBrushSize)
    Q_PROPERTY(qreal brushHardness    READ brushHardness    WRITE setBrushHardness)

public:
    explicit AppSettings(QObject *parent = nullptr);

    AppSettings(const AppSettings&) = delete;
    AppSettings(AppSettings&&) noexcept = delete;

    AppSettings& operator=(const AppSettings&) = delete;
    AppSettings& operator=(AppSettings&&) noexcept = delete;

    ~AppSettings() noexcept override = default;

    bool disableAds() const;
    void setDisableAds(bool disable);

    int defaultBrushSize() const;
    void setDefaultBrushSize(int size);

    int brushSize() const;
    void setBrushSize(int size);

    qreal brushHardness() const;
    void setBrushHardness(qreal hardness);

private:
    int                        DefaultBrushSize;
    std::shared_ptr<QSettings> Settings;
};

#endif // APPSETTINGS_H
