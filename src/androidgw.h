#ifndef ANDROIDGW_H
#define ANDROIDGW_H

#include <QtCore/QObject>
#include <QtCore/QString>

class AndroidGW : public QObject
{
    Q_OBJECT

private:
    explicit AndroidGW(QObject *parent = nullptr);
    ~AndroidGW() noexcept override = default;

public:
    AndroidGW(const AndroidGW &) = delete;
    AndroidGW(AndroidGW &&) noexcept = delete;

    AndroidGW &operator=(const AndroidGW &) = delete;
    AndroidGW &operator=(AndroidGW &&) noexcept = delete;

    static AndroidGW &GetInstance();

signals:
    void setBannerViewHeight(int height);

    void processImageSelection(const QString &image_file, int image_orientation);
    void processImageSelectionCancel();
    void processImageSelectionFailure();
};

#endif // ANDROIDGW_H
