#ifndef TRIALMANAGER_H
#define TRIALMANAGER_H

#include <QtCore/QObject>

class TrialManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool trialMode READ trialMode)

public:
    explicit TrialManager(QObject *parent = 0);
    virtual ~TrialManager();

    bool trialMode() const;
};

#endif // TRIALMANAGER_H
