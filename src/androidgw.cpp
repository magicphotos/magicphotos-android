#include <QtAndroidExtras/QAndroidJniObject>

#include "androidgw.h"

#define JAVA_NATIVE_METHOD_NAME(class_name, method_name) Java_com_derevenetz_oleg_magicphotos_ ## class_name ## _ ## method_name

AndroidGW::AndroidGW(QObject *parent) :
    QObject(parent)
{
}

AndroidGW &AndroidGW::GetInstance()
{
    static AndroidGW instance;

    return instance;
}

extern "C" JNIEXPORT void JNICALL JAVA_NATIVE_METHOD_NAME(MagicActivity, deviceConfigurationUpdated)(JNIEnv *, jclass)
{
    emit AndroidGW::GetInstance().deviceConfigurationUpdated();
}

extern "C" JNIEXPORT void JNICALL JAVA_NATIVE_METHOD_NAME(MagicActivity, bannerViewHeightUpdated)(JNIEnv *, jclass, jint height)
{
    emit AndroidGW::GetInstance().bannerViewHeightUpdated(height);
}

extern "C" JNIEXPORT void JNICALL JAVA_NATIVE_METHOD_NAME(MagicActivity, imageSelected)(JNIEnv *jni_env, jclass, jstring j_image_path, jint image_orientation)
{
    const char *str        = jni_env->GetStringUTFChars(j_image_path, nullptr);
    QString     image_path = QString::fromUtf8(str);

    jni_env->ReleaseStringUTFChars(j_image_path, str);

    emit AndroidGW::GetInstance().imageSelected(image_path, image_orientation);
}

extern "C" JNIEXPORT void JNICALL JAVA_NATIVE_METHOD_NAME(MagicActivity, imageSelectionCancelled)(JNIEnv *)
{
    emit AndroidGW::GetInstance().imageSelectionCancelled();
}

extern "C" JNIEXPORT void JNICALL JAVA_NATIVE_METHOD_NAME(MagicActivity, imageSelectionFailed)(JNIEnv *)
{
    emit AndroidGW::GetInstance().imageSelectionFailed();
}
