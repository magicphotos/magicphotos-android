#ifndef UIHELPER_H
#define UIHELPER_H

#include <QtCore/QObject>

class UIHelper : public QObject
{
    Q_OBJECT

public:
    explicit UIHelper(QObject *parent = nullptr);
    virtual ~UIHelper();

    Q_INVOKABLE int getScreenDPI();
    Q_INVOKABLE QString getSaveDirectory();

    Q_INVOKABLE bool requestReadStoragePermission();
    Q_INVOKABLE bool requestWriteStoragePermission();

    Q_INVOKABLE void showGallery();
    Q_INVOKABLE void refreshGallery(QString image_file);
    Q_INVOKABLE void shareImage(QString image_file);

signals:
    void imageSelected(QString image_file, int image_orientation);
    void imageSelectionCancelled();
    void imageSelectionFailed();

public slots:
    void processImageSelection(QString image_file, int image_orientation);
    void processImageSelectionCancel();
    void processImageSelectionFailure();
};

#endif // UIHELPER_H
