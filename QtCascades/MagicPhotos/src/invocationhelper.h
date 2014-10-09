#ifndef INVOCATIONHELPER_H
#define INVOCATIONHELPER_H

#include <QtCore/QObject>
#include <QtCore/QString>

#include <bb/system/InvokeRequest>
#include <bb/system/InvokeManager>

class InvocationHelper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString imageFile READ imageFile WRITE setImageFile NOTIFY imageFileChanged)

public:
    explicit InvocationHelper(QObject *parent = 0);
    virtual ~InvocationHelper();

    QString imageFile() const;
    void    setImageFile(const QString &image_file);

private slots:
    void invoked(const bb::system::InvokeRequest &request);

signals:
    void imageFileChanged();

private:
    QString                   ImageFile;
    bb::system::InvokeManager *InvocationManager;
};

#endif // INVOCATIONHELPER_H
