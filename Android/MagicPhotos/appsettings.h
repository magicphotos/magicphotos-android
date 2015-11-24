#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QtCore/qmath.h>
#include <QtCore/QObject>
#include <QtCore/QSettings>

class AppSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int   defaultBrushSize         READ defaultBrushSize         WRITE setDefaultBrushSize)
    Q_PROPERTY(int   brushSize                READ brushSize                WRITE setBrushSize)
    Q_PROPERTY(bool  showModeChangeSuggestion READ showModeChangeSuggestion WRITE setShowModeChangeSuggestion)
    Q_PROPERTY(bool  showPromoPopup           READ showPromoPopup           WRITE setShowPromoPopup)
    Q_PROPERTY(qreal brushOpacity             READ brushOpacity             WRITE setBrushOpacity)

public:
    explicit AppSettings(QObject *parent = 0);
    virtual ~AppSettings();

    int  defaultBrushSize() const;
    void setDefaultBrushSize(const int &size);

    int  brushSize() const;
    void setBrushSize(const int &size);

    bool showModeChangeSuggestion() const;
    void setShowModeChangeSuggestion(const bool &show);

    bool showPromoPopup() const;
    void setShowPromoPopup(const bool &show);

    qreal brushOpacity() const;
    void  setBrushOpacity(const qreal &opacity);

private:
    int        DefaultBrushSize;
    QSettings *Settings;
};

#endif // APPSETTINGS_H
