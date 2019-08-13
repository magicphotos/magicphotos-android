#ifndef UIHELPER_H
#define UIHELPER_H

#include <QtCore/QObject>
#include <QtCore/QString>

class UIHelper : public QObject
{
    Q_OBJECT

private:
    explicit UIHelper(QObject *parent = nullptr);
    ~UIHelper() noexcept override = default;

public:
    UIHelper(const UIHelper &) = delete;
    UIHelper(UIHelper &&) noexcept = delete;

    UIHelper &operator=(const UIHelper &) = delete;
    UIHelper &operator=(UIHelper &&) noexcept = delete;

    static UIHelper &GetInstance();

    Q_INVOKABLE int getScreenDPI();
    Q_INVOKABLE QString getSaveDirectory();

    Q_INVOKABLE bool requestReadStoragePermission();
    Q_INVOKABLE bool requestWriteStoragePermission();

    Q_INVOKABLE void showGallery();
    Q_INVOKABLE void refreshGallery(const QString &image_file);
    Q_INVOKABLE void shareImage(const QString &image_file);

public slots:
    void handleImageSelection(const QString &image_file, int image_orientation);
    void handleImageSelectionCancel();
    void handleImageSelectionFailure();

signals:
    void imageSelected(const QString &imageFile, int imageOrientation);
    void imageSelectionCancelled();
    void imageSelectionFailed();
};

#endif // UIHELPER_H
