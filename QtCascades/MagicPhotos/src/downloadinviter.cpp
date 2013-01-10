#include <QtCore/QString>
#include <QtCore/QUuid>

#include <bb/platform/bbm/MessageService>

#include "downloadinviter.h"

DownloadInviter::DownloadInviter() : bb::cascades::CustomControl()
{
    State      = InviterStateNotStarted;
    BBMContext = NULL;
}

DownloadInviter::~DownloadInviter()
{
    if (BBMContext != NULL) {
        delete BBMContext;
    }
}

void DownloadInviter::sendDownloadInvitation()
{
    State = InviterStateStarted;

    if (BBMContext == NULL) {
        BBMContext = new bb::platform::bbm::Context(QUuid(QString("c0e86694-0ce4-4eea-b343-4ba005c7a4fa")), this);

        QObject::connect(BBMContext, SIGNAL(registrationStateUpdated(bb::platform::bbm::RegistrationState::Type)), this, SLOT(contextRegistrationStateUpdated(bb::platform::bbm::RegistrationState::Type)));
    }

    contextRegistrationStateUpdated(BBMContext->registrationState());
}

void DownloadInviter::contextRegistrationStateUpdated(bb::platform::bbm::RegistrationState::Type state)
{
    if (State == InviterStateStarted) {
        if (BBMContext->isAccessAllowed()) {
            State = InviterStateNotStarted;

            bb::platform::bbm::MessageService message_service(BBMContext);

            if (message_service.sendDownloadInvitation()) {
                emit invitationSent();
            } else {
                emit invitationSendFailed();
            }
        } else if (state != bb::platform::bbm::RegistrationState::Unknown) {
            if (BBMContext->requestRegisterApplication()) {
                State = InviterStateRegPending;
            } else {
                State = InviterStateNotStarted;

                emit invitationSendFailed();
            }
        }
    } else if (State == InviterStateRegPending) {
        if (BBMContext->isAccessAllowed()) {
            State = InviterStateNotStarted;

            bb::platform::bbm::MessageService message_service(BBMContext);

            if (message_service.sendDownloadInvitation()) {
                emit invitationSent();
            } else {
                emit invitationSendFailed();
            }
        } else if (state != bb::platform::bbm::RegistrationState::Pending) {
            State = InviterStateNotStarted;

            emit invitationSendFailed();
        }
    }
}
