#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QtCore/QObject>
#include <QtCore/QSettings>

class AppSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool showModeChangeSuggestion READ showModeChangeSuggestion WRITE setShowModeChangeSuggestion)

public:
    explicit AppSettings(QObject *parent = 0);
    virtual ~AppSettings();

    bool showModeChangeSuggestion() const;
    void setShowModeChangeSuggestion(const bool &show);

private:
    QSettings *Settings;
};

#endif // APPSETTINGS_H
