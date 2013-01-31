#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QtCore/QObject>
#include <QtCore/QString>

#include <bb/system/SystemToast>

class MessageBox : public QObject
{
    Q_OBJECT

    Q_ENUMS(DialogResult)

public:
    explicit MessageBox(QObject *parent = 0);
    virtual ~MessageBox();

    Q_INVOKABLE void showToast(const QString &body);

private:
    bb::system::SystemToast Toast;
};

#endif // MESSAGEBOX_H
