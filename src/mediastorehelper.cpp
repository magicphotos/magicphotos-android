#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtAndroidExtras/QtAndroid>
#include <QtAndroidExtras/QAndroidJniObject>

#include "mediastorehelper.h"

MediaStoreHelper::MediaStoreHelper(QObject *parent) : QObject(parent)
{
}

MediaStoreHelper &MediaStoreHelper::GetInstance()
{
    static MediaStoreHelper instance;

    return instance;
}

QString MediaStoreHelper::imageFilePath() const
{
    QString tmp_dir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    if (tmp_dir != QStringLiteral("")) {
        QDir().mkpath(tmp_dir);
    }

    return QDir(tmp_dir).filePath(QStringLiteral("output.jpg"));
}

bool MediaStoreHelper::addImageToMediaStore(const QString &image_path)
{
    QAndroidJniObject j_image_path = QAndroidJniObject::fromString(image_path);

    return QtAndroid::androidActivity().callMethod<jboolean>("addImageToMediaStore", "(Ljava/lang/String;)Z", j_image_path.object<jstring>());
}
