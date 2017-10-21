#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QtCore/qmath.h>
#include <QtCore/QObject>
#include <QtCore/QSettings>

class AppSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int   defaultBrushSize READ defaultBrushSize WRITE setDefaultBrushSize)
    Q_PROPERTY(int   brushSize        READ brushSize        WRITE setBrushSize)
    Q_PROPERTY(bool  isFullVersion    READ isFullVersion    WRITE setIsFullVersion)
    Q_PROPERTY(qreal brushOpacity     READ brushOpacity     WRITE setBrushOpacity)

public:
    explicit AppSettings(QObject *parent = 0);
    virtual ~AppSettings();

    int  defaultBrushSize() const;
    void setDefaultBrushSize(const int &size);

    int  brushSize() const;
    void setBrushSize(const int &size);

    bool isFullVersion() const;
    void setIsFullVersion(const bool &full);

    qreal brushOpacity() const;
    void  setBrushOpacity(const qreal &opacity);

private:
    int        DefaultBrushSize;
    QSettings *Settings;
};

#endif // APPSETTINGS_H
