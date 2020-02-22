#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtAndroidExtras/QtAndroid>
#include <QtAndroidExtras/QAndroidJniObject>

#include "uihelper.h"

UIHelper::UIHelper(QObject *parent) : QObject(parent)
{
    DarkTheme = QtAndroid::androidActivity().callMethod<jboolean>("getNightModeStatus");
    ScreenDpi = QtAndroid::androidActivity().callMethod<jint>("getScreenDpi");
}

UIHelper &UIHelper::GetInstance()
{
    static UIHelper instance;

    return instance;
}

bool UIHelper::darkTheme() const
{
    return DarkTheme;
}

int UIHelper::screenDpi() const
{
    return ScreenDpi;
}

QString UIHelper::getSaveImageFilePath()
{
    QString tmp_dir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    if (tmp_dir != QStringLiteral("")) {
        QDir().mkpath(tmp_dir);
    }

    return QDir(tmp_dir).filePath(QStringLiteral("output.jpg"));
}

bool UIHelper::requestReadStoragePermission()
{
    if (QtAndroid::checkPermission(QStringLiteral("android.permission.READ_EXTERNAL_STORAGE")) == QtAndroid::PermissionResult::Granted) {
        return true;
    } else {
        QtAndroid::PermissionResultMap result = QtAndroid::requestPermissionsSync(QStringList(QStringLiteral("android.permission.READ_EXTERNAL_STORAGE")));

        return (result.contains(QStringLiteral("android.permission.READ_EXTERNAL_STORAGE")) &&
                result[QStringLiteral("android.permission.READ_EXTERNAL_STORAGE")] == QtAndroid::PermissionResult::Granted);
    }
}

bool UIHelper::requestWriteStoragePermission()
{
    if (QtAndroid::checkPermission(QStringLiteral("android.permission.WRITE_EXTERNAL_STORAGE")) == QtAndroid::PermissionResult::Granted) {
        return true;
    } else {
        QtAndroid::PermissionResultMap result = QtAndroid::requestPermissionsSync(QStringList(QStringLiteral("android.permission.WRITE_EXTERNAL_STORAGE")));

        return (result.contains(QStringLiteral("android.permission.WRITE_EXTERNAL_STORAGE")) &&
                result[QStringLiteral("android.permission.WRITE_EXTERNAL_STORAGE")] == QtAndroid::PermissionResult::Granted);
    }
}

void UIHelper::showGallery()
{
    QtAndroid::androidActivity().callMethod<void>("showGallery");
}

bool UIHelper::addImageToMediaLibrary(const QString &image_file)
{
    QAndroidJniObject j_image_file = QAndroidJniObject::fromString(image_file);

    return QtAndroid::androidActivity().callMethod<jboolean>("addImageToMediaLibrary", "(Ljava/lang/String;)Z", j_image_file.object<jstring>());
}

void UIHelper::shareImage(const QString &image_file)
{
    QAndroidJniObject j_image_file = QAndroidJniObject::fromString(image_file);

    QtAndroid::androidActivity().callMethod<void>("shareImage", "(Ljava/lang/String;)V", j_image_file.object<jstring>());
}

void UIHelper::handleDeviceConfigurationUpdate()
{
    int screen_dpi = QtAndroid::androidActivity().callMethod<jint>("getScreenDpi");

    if (ScreenDpi != screen_dpi) {
        ScreenDpi = screen_dpi;

        emit screenDpiChanged(ScreenDpi);
    }

    bool dark_theme = QtAndroid::androidActivity().callMethod<jboolean>("getNightModeStatus");

    if (DarkTheme != dark_theme) {
        DarkTheme = dark_theme;

        emit darkThemeChanged(DarkTheme);
    }
}

void UIHelper::handleImageSelection(const QString &image_file, int image_orientation)
{
    emit imageSelected(image_file, image_orientation);
}

void UIHelper::handleImageSelectionCancel()
{
    emit imageSelectionCancelled();
}

void UIHelper::handleImageSelectionFailure()
{
    emit imageSelectionFailed();
}
