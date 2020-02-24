#ifndef MEDIASTOREHELPER_H
#define MEDIASTOREHELPER_H

#include <QtCore/QObject>
#include <QtCore/QString>

class MediaStoreHelper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString imageFilePath READ imageFilePath)

private:
    explicit MediaStoreHelper(QObject *parent = nullptr);
    ~MediaStoreHelper() noexcept override = default;

public:
    MediaStoreHelper(const MediaStoreHelper &) = delete;
    MediaStoreHelper(MediaStoreHelper &&) noexcept = delete;

    MediaStoreHelper &operator=(const MediaStoreHelper &) = delete;
    MediaStoreHelper &operator=(MediaStoreHelper &&) noexcept = delete;

    static MediaStoreHelper &GetInstance();

    QString imageFilePath() const;

    Q_INVOKABLE bool addImageToMediaStore(const QString &image_path) const;
};

#endif // MEDIASTOREHELPER_H
