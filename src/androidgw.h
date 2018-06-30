#ifndef ANDROIDGW_H
#define ANDROIDGW_H

#include <QtCore/QObject>
#include <QtCore/QString>

class AndroidGW : public QObject
{
    Q_OBJECT

public:
    explicit AndroidGW(QObject *parent = 0);

    static AndroidGW *instance();

signals:
    void setBannerViewHeight(int height);

    void processImageSelection(QString image_file, int image_orientation);
    void processImageSelectionCancel();
    void processImageSelectionFailure();

private:
    static AndroidGW *Instance;
};

#endif // ANDROIDGW_H
