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
    void deviceConfigurationChanged();
    void bannerViewHeightChanged(int bannerViewHeight);
    void imageSelected(const QString &imageFile, int imageOrientation);
    void imageSelectionCancelled();
    void imageSelectionFailed();
};

#endif // ANDROIDGW_H
