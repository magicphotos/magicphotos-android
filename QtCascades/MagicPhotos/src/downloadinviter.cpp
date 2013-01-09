#include <QtCore/QString>
#include <QtCore/QUuid>
#include <bb/platform/bbm/MessageService>

#include "downloadinviter.h"

DownloadInviter::DownloadInviter() : bb::cascades::CustomControl()
{
    RegistrationState = RegStateNotStarted;
    BBMContext        = NULL;
}

DownloadInviter::~DownloadInviter()
{
    if (BBMContext != NULL) {
        delete BBMContext;
    }
}

void DownloadInviter::sendDownloadInvitation()
{
    RegistrationState = RegStateStarted;

    if (BBMContext == NULL) {
        BBMContext = new bb::platform::bbm::Context(QUuid(QString("c0e86694-0ce4-4eea-b343-4ba005c7a4fa")), this);

        QObject::connect(BBMContext, SIGNAL(registrationStateUpdated(bb::platform::bbm::RegistrationState::Type)), this, SLOT(contextRegistrationStateUpdated(bb::platform::bbm::RegistrationState::Type)));
    }

    contextRegistrationStateUpdated(BBMContext->registrationState());
}

void DownloadInviter::contextRegistrationStateUpdated(bb::platform::bbm::RegistrationState::Type state)
{
    if (BBMContext->isAccessAllowed()) {
        RegistrationState = RegStateRegistered;

        bb::platform::bbm::MessageService message_service(BBMContext);

        if (message_service.sendDownloadInvitation()) {
            emit invitationSent();
        } else {
            emit invitationSendFailed();
        }
    } else {
        if (RegistrationState == RegStateStarted) {
            if (state != bb::platform::bbm::RegistrationState::Unknown) {
                if (BBMContext->requestRegisterApplication()) {
                    RegistrationState = RegStatePending;
                } else {
                    RegistrationState = RegStateError;

                    emit invitationSendFailed();
                }
            }
        } else if (RegistrationState == RegStatePending) {
            if (state != bb::platform::bbm::RegistrationState::Pending) {
                RegistrationState = RegStateError;

                emit invitationSendFailed();
            }
        }
    }
}
