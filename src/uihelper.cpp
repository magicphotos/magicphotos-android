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
