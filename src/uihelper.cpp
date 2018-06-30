#include <QtAndroidExtras/QtAndroid>
#include <QtAndroidExtras/QAndroidJniObject>

#include "uihelper.h"

UIHelper::UIHelper(QObject *parent) : QObject(parent)
{
}

UIHelper::~UIHelper()
{
}

int UIHelper::getScreenDPI()
{
    return QtAndroid::androidActivity().callMethod<jint>("getScreenDPI");
}

QString UIHelper::getSaveDirectory()
{
    QAndroidJniObject str_object = QtAndroid::androidActivity().callObjectMethod<jstring>("getSaveDirectory");

    return str_object.toString();
}

bool UIHelper::requestReadStoragePermission()
{
    if (QtAndroid::checkPermission("android.permission.READ_EXTERNAL_STORAGE") == QtAndroid::PermissionResult::Granted) {
        return true;
    } else {
        QtAndroid::PermissionResultMap result = QtAndroid::requestPermissionsSync(QStringList("android.permission.READ_EXTERNAL_STORAGE"));

        if (result.contains("android.permission.READ_EXTERNAL_STORAGE") && result["android.permission.READ_EXTERNAL_STORAGE"] == QtAndroid::PermissionResult::Granted) {
            return true;
        } else {
            return false;
        }
    }
}

bool UIHelper::requestWriteStoragePermission()
{
    if (QtAndroid::checkPermission("android.permission.WRITE_EXTERNAL_STORAGE") == QtAndroid::PermissionResult::Granted) {
        return true;
    } else {
        QtAndroid::PermissionResultMap result = QtAndroid::requestPermissionsSync(QStringList("android.permission.WRITE_EXTERNAL_STORAGE"));

        if (result.contains("android.permission.WRITE_EXTERNAL_STORAGE") && result["android.permission.WRITE_EXTERNAL_STORAGE"] == QtAndroid::PermissionResult::Granted) {
            return true;
        } else {
            return false;
        }
    }
}

void UIHelper::showGallery()
{
    QtAndroid::androidActivity().callMethod<void>("showGallery");
}

void UIHelper::refreshGallery(QString image_file)
{
    QAndroidJniObject j_image_file = QAndroidJniObject::fromString(image_file);

    QtAndroid::androidActivity().callMethod<void>("refreshGallery", "(Ljava/lang/String;)V", j_image_file.object<jstring>());
}

void UIHelper::shareImage(QString image_file)
{
    QAndroidJniObject j_image_file = QAndroidJniObject::fromString(image_file);

    QtAndroid::androidActivity().callMethod<void>("shareImage", "(Ljava/lang/String;)V", j_image_file.object<jstring>());
}

void UIHelper::processImageSelection(QString image_file, int image_orientation)
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
