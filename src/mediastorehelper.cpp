#include <QtCore/QLatin1String>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtAndroidExtras/QtAndroid>
#include <QtAndroidExtras/QAndroidJniObject>

#include "mediastorehelper.h"

MediaStoreHelper::MediaStoreHelper(QObject *parent) :
    QObject(parent)
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

    if (tmp_dir != QLatin1String("")) {
        QDir().mkpath(tmp_dir);
    }

    return QDir(tmp_dir).filePath(QStringLiteral("output.jpg"));
}

bool MediaStoreHelper::addImageToMediaStore(const QString &image_path) const
{
    QAndroidJniObject j_image_path = QAndroidJniObject::fromString(image_path);

    return QtAndroid::androidActivity().callMethod<jboolean>("addImageToMediaStore", "(Ljava/lang/String;)Z", j_image_path.object<jstring>());
}
