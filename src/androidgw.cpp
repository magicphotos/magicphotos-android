#include <QtAndroidExtras/QAndroidJniObject>

#include "androidgw.h"

#define JAVA_NATIVE_METHOD_NAME(class_name, method_name) Java_com_derevenetz_oleg_magicphotos_stdalone_ ## class_name ## _ ## method_name

AndroidGW *AndroidGW::Instance = nullptr;

AndroidGW::AndroidGW(QObject *parent) : QObject(parent)
{
    Instance = this;
}

AndroidGW *AndroidGW::instance()
{
    return Instance;
}

extern "C" JNIEXPORT void JNICALL JAVA_NATIVE_METHOD_NAME(MagicActivity, bannerViewHeightUpdated)(JNIEnv *, jclass, jint height)
{
    emit AndroidGW::instance()->setBannerViewHeight(height);
}

extern "C" JNIEXPORT void JNICALL JAVA_NATIVE_METHOD_NAME(MagicActivity, imageSelected)(JNIEnv *jni_env, jclass, jstring j_image_file, jint image_orientation)
{
    const char* str        = jni_env->GetStringUTFChars(j_image_file, nullptr);
    QString     image_file = str;

    jni_env->ReleaseStringUTFChars(j_image_file, str);

    emit AndroidGW::instance()->processImageSelection(image_file, image_orientation);
}

extern "C" JNIEXPORT void JNICALL JAVA_NATIVE_METHOD_NAME(MagicActivity, imageSelectionCancelled)(JNIEnv *)
{
    emit AndroidGW::instance()->processImageSelectionCancel();
}

extern "C" JNIEXPORT void JNICALL JAVA_NATIVE_METHOD_NAME(MagicActivity, imageSelectionFailed)(JNIEnv *)
{
    emit AndroidGW::instance()->processImageSelectionFailure();
}
