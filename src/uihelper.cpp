#include <QtAndroidExtras/QtAndroid>
#include <QtAndroidExtras/QAndroidJniObject>

#include "uihelper.h"

UIHelper::UIHelper(QObject *parent) : QObject(parent)
{
}

int UIHelper::getScreenDPI()
{
    return QtAndroid::androidActivity().callMethod<jint>("getScreenDPI");
}

QString UIHelper::getSaveDirectory()
{
    QAndroidJniObject j_save_directory = QtAndroid::androidActivity().callObjectMethod<jstring>("getSaveDirectory");

    return j_save_directory.toString();
}

bool UIHelper::requestReadStoragePermission()
{
    if (QtAndroid::checkPermission("android.permission.READ_EXTERNAL_STORAGE") == QtAndroid::PermissionResult::Granted) {
        return true;
    } else {
        QtAndroid::PermissionResultMap result = QtAndroid::requestPermissionsSync(QStringList("android.permission.READ_EXTERNAL_STORAGE"));

        return (result.contains("android.permission.READ_EXTERNAL_STORAGE") && result["android.permission.READ_EXTERNAL_STORAGE"] == QtAndroid::PermissionResult::Granted);
    }
}

bool UIHelper::requestWriteStoragePermission()
{
    if (QtAndroid::checkPermission("android.permission.WRITE_EXTERNAL_STORAGE") == QtAndroid::PermissionResult::Granted) {
        return true;
    } else {
        QtAndroid::PermissionResultMap result = QtAndroid::requestPermissionsSync(QStringList("android.permission.WRITE_EXTERNAL_STORAGE"));

        return (result.contains("android.permission.WRITE_EXTERNAL_STORAGE") && result["android.permission.WRITE_EXTERNAL_STORAGE"] == QtAndroid::PermissionResult::Granted);
    }
}

void UIHelper::showGallery()
{
    QtAndroid::androidActivity().callMethod<void>("showGallery");
}

void UIHelper::refreshGallery(const QString &image_file)
{
    QAndroidJniObject j_image_file = QAndroidJniObject::fromString(image_file);

    QtAndroid::androidActivity().callMethod<void>("refreshGallery", "(Ljava/lang/String;)V", j_image_file.object<jstring>());
}

void UIHelper::shareImage(const QString &image_file)
{
    QAndroidJniObject j_image_file = QAndroidJniObject::fromString(image_file);

    QtAndroid::androidActivity().callMethod<void>("shareImage", "(Ljava/lang/String;)V", j_image_file.object<jstring>());
}

void UIHelper::processImageSelection(const QString &image_file, int image_orientation)
{
    emit imageSelected(image_file, image_orientation);
}

void UIHelper::processImageSelectionCancel()
{
    emit imageSelectionCancelled();
}

void UIHelper::processImageSelectionFailure()
{
    emit imageSelectionFailed();
}
