#ifndef ANDROIDGW_H
#define ANDROIDGW_H

#include <QtCore/QObject>
#include <QtAndroidExtras/QAndroidJniObject>

class AndroidGW : public QObject
{
    Q_OBJECT

public:
    explicit AndroidGW(QObject *parent = 0);

    static AndroidGW *instance();

    Q_INVOKABLE int  getScreenDPI();
    Q_INVOKABLE void showGallery();
    Q_INVOKABLE void refreshGallery(const QString &image_file);

signals:
    void imageSelected(const QString &image_file, const int &image_orientation);
    void imageSelectionCancelled();

private:
    static AndroidGW *Instance;
};

#endif // ANDROIDGW_H
