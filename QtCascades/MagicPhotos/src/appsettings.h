#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QtCore/qmath.h>
#include <QtCore/QObject>
#include <QtCore/QSettings>

class AppSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int   launchNumber                     READ launchNumber                     WRITE setLaunchNumber)
    Q_PROPERTY(int   lastFeedbackRequestLaunchNumber  READ lastFeedbackRequestLaunchNumber  WRITE setLastFeedbackRequestLaunchNumber)
    Q_PROPERTY(int   defaultBrushSize                 READ defaultBrushSize                 WRITE setDefaultBrushSize)
    Q_PROPERTY(int   brushSize                        READ brushSize                        WRITE setBrushSize)
    Q_PROPERTY(bool  requestFeedback                  READ requestFeedback                  WRITE setRequestFeedback)
    Q_PROPERTY(bool  showModeChangeSuggestion         READ showModeChangeSuggestion         WRITE setShowModeChangeSuggestion)
    Q_PROPERTY(qreal brushOpacity                     READ brushOpacity                     WRITE setBrushOpacity)
    Q_PROPERTY(qreal imageResolutionLimit             READ imageResolutionLimit             WRITE setImageResolutionLimit)

public:
    explicit AppSettings(QObject *parent = 0);
    virtual ~AppSettings();

    int  launchNumber() const;
    void setLaunchNumber(const int &number);

    int  lastFeedbackRequestLaunchNumber() const;
    void setLastFeedbackRequestLaunchNumber(const int &number);

    int  defaultBrushSize() const;
    void setDefaultBrushSize(const int &size);

    int  brushSize() const;
    void setBrushSize(const int &size);

    bool requestFeedback() const;
    void setRequestFeedback(const bool &request);

    bool showModeChangeSuggestion() const;
    void setShowModeChangeSuggestion(const bool &show);

    qreal brushOpacity() const;
    void  setBrushOpacity(const qreal &opacity);

    qreal imageResolutionLimit() const;
    void  setImageResolutionLimit(const qreal &limit);

private:
    int        DefaultBrushSize;
    QSettings *Settings;
};

#endif // APPSETTINGS_H
