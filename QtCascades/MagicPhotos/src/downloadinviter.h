#ifndef DOWNLOADINVITER_H
#define DOWNLOADINVITER_H

#include <QtCore/QObject>

#include <bb/platform/bbm/Context>
#include <bb/cascades/CustomControl>

class DownloadInviter : public bb::cascades::CustomControl
{
    Q_OBJECT

public:
    explicit DownloadInviter();
    virtual ~DownloadInviter();

    Q_INVOKABLE void sendDownloadInvitation();

public slots:
    void contextRegistrationStateUpdated(bb::platform::bbm::RegistrationState::Type state);

signals:
    void invitationSent();
    void invitationSendFailed();

private:
    enum InviterState {
        InviterStateNotStarted,
        InviterStateStarted,
        InviterStateRegPending,
    };

    int                        State;
    bb::platform::bbm::Context *BBMContext;
};

#endif // DOWNLOADINVITER_H
