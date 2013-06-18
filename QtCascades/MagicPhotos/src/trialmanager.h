#ifndef TRIALMANAGER_H
#define TRIALMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QString>

class TrialManager : public QObject
{
    Q_OBJECT

public:
    explicit TrialManager(QObject *parent = 0);
    virtual ~TrialManager();

private:
};

#endif // TRIALMANAGER_H
