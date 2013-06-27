#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QtCore/QObject>
#include <QtCore/QSettings>

class AppSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int  launchNumber            READ launchNumber            WRITE setLaunchNumber)
    Q_PROPERTY(int  lastRequestLaunchNumber READ lastRequestLaunchNumber WRITE setLastRequestLaunchNumber)
    Q_PROPERTY(bool requestFeedback         READ requestFeedback         WRITE setRequestFeedback)

public:
    explicit AppSettings(QObject *parent = 0);
    virtual ~AppSettings();

    int  launchNumber() const;
    void setLaunchNumber(const int &number);

    int  lastRequestLaunchNumber() const;
    void setLastRequestLaunchNumber(const int &number);

    bool requestFeedback() const;
    void setRequestFeedback(const bool &request);

private:
    QSettings *Settings;
};

#endif // APPSETTINGS_H
