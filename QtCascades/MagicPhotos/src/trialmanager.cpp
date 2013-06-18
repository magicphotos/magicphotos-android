#include "trialmanager.h"

TrialManager::TrialManager(QObject *parent) : QObject(parent)
{
}

TrialManager::~TrialManager()
{
}

bool TrialManager::trialMode() const
{
#ifdef TRIAL_VERSION
    return true;
#else
    return false;
#endif
}
