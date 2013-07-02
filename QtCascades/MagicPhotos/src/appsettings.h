#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QtCore/QObject>
#include <QtCore/QSettings>

class AppSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int  launchNumber                     READ launchNumber                     WRITE setLaunchNumber)
    Q_PROPERTY(int  lastFeedbackRequestLaunchNumber  READ lastFeedbackRequestLaunchNumber  WRITE setLastFeedbackRequestLaunchNumber)
    Q_PROPERTY(bool requestFeedback                  READ requestFeedback                  WRITE setRequestFeedback)
    Q_PROPERTY(bool showModeChangeSuggestion         READ showModeChangeSuggestion         WRITE setShowModeChangeSuggestion)

public:
    explicit AppSettings(QObject *parent = 0);
    virtual ~AppSettings();

    int  launchNumber() const;
    void setLaunchNumber(const int &number);

    int  lastFeedbackRequestLaunchNumber() const;
    void setLastFeedbackRequestLaunchNumber(const int &number);

    bool requestFeedback() const;
    void setRequestFeedback(const bool &request);

    bool showModeChangeSuggestion() const;
    void setShowModeChangeSuggestion(const bool &show);

private:
    QSettings *Settings;
};

#endif // APPSETTINGS_H
