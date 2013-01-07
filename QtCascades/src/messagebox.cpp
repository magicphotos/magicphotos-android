#include <bb/system/SystemUiResult>
#include <bb/system/SystemDialog>

#include "messagebox.h"

MessageBox::MessageBox(QObject *parent) : QObject(parent)
{
}

MessageBox::~MessageBox()
{
}

int MessageBox::showMessage(const QString &title, const QString &body, const QString &confirmButtonLabel)
{
	bb::system::SystemDialog dialog(confirmButtonLabel);

	dialog.setTitle(title);
	dialog.setBody(body);

	int result = dialog.exec();

	switch (result) {
	case bb::system::SystemUiResult::ButtonSelection:
		return DialogResultButtonSelection;
	case bb::system::SystemUiResult::ConfirmButtonSelection:
		return DialogResultConfirmButtonSelection;
	case bb::system::SystemUiResult::CancelButtonSelection:
		return DialogResultCancelButtonSelection;
	case bb::system::SystemUiResult::TimeOut:
		return DialogResultTimeOut;
	case bb::system::SystemUiResult::Error:
		return DialogResultError;
	case bb::system::SystemUiResult::CustomButtonSelection:
		return DialogResultCustomButtonSelection;
	default:
		return DialogResultNone;
	}
}

int MessageBox::showQuestion(const QString &title, const QString &body, const QString &confirmButtonLabel, const QString &cancelButtonLabel)
{
	bb::system::SystemDialog dialog(confirmButtonLabel, cancelButtonLabel);

	dialog.setTitle(title);
	dialog.setBody(body);

	int result = dialog.exec();

	switch (result) {
	case bb::system::SystemUiResult::ButtonSelection:
		return DialogResultButtonSelection;
	case bb::system::SystemUiResult::ConfirmButtonSelection:
		return DialogResultConfirmButtonSelection;
	case bb::system::SystemUiResult::CancelButtonSelection:
		return DialogResultCancelButtonSelection;
	case bb::system::SystemUiResult::TimeOut:
		return DialogResultTimeOut;
	case bb::system::SystemUiResult::Error:
		return DialogResultError;
	case bb::system::SystemUiResult::CustomButtonSelection:
		return DialogResultCustomButtonSelection;
	default:
		return DialogResultNone;
	}
}
