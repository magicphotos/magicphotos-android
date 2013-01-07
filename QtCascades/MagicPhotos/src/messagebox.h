#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QtCore/QObject>
#include <QtCore/QString>

class MessageBox : public QObject
{
    Q_OBJECT

    Q_ENUMS(DialogResult)

public:
    explicit MessageBox(QObject *parent = 0);
    virtual ~MessageBox();

    Q_INVOKABLE int showMessage(const QString &title, const QString &body, const QString &confirmButtonLabel);
    Q_INVOKABLE int showQuestion(const QString &title, const QString &body, const QString &confirmButtonLabel, const QString &cancelButtonLabel);

    enum DialogResult {
    	DialogResultNone,
    	DialogResultButtonSelection,
    	DialogResultConfirmButtonSelection,
    	DialogResultCancelButtonSelection,
    	DialogResultTimeOut,
    	DialogResultError,
    	DialogResultCustomButtonSelection
    };
};

#endif // MESSAGEBOX_H
