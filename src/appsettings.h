#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QSettings>

class AppSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int   defaultBrushSize READ defaultBrushSize WRITE setDefaultBrushSize)
    Q_PROPERTY(int   brushSize        READ brushSize        WRITE setBrushSize)
    Q_PROPERTY(bool  disableAds       READ disableAds       WRITE setDisableAds)
    Q_PROPERTY(qreal brushOpacity     READ brushOpacity     WRITE setBrushOpacity)

public:
    explicit AppSettings(QObject *parent = nullptr);
    ~AppSettings() override;

    int defaultBrushSize() const;
    void setDefaultBrushSize(int size);

    int brushSize() const;
    void setBrushSize(int size);

    bool disableAds() const;
    void setDisableAds(bool disable);

    qreal brushOpacity() const;
    void setBrushOpacity(qreal opacity);

private:
    int        DefaultBrushSize;
    QSettings *Settings;
};

#endif // APPSETTINGS_H
