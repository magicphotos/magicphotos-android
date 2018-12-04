#include <QtAndroidExtras/QAndroidJniObject>

#include "androidgw.h"

AndroidGW *AndroidGW::Instance = nullptr;

AndroidGW::AndroidGW(QObject *parent) : QObject(parent)
{
    Instance = this;
}

AndroidGW *AndroidGW::instance()
{
    return Instance;
}

extern "C" JNIEXPORT void JNICALL Java_com_derevenetz_oleg_magicphotos_stdalone_MagicActivity_bannerViewHeightUpdated(JNIEnv *, jclass, jint height)
{
    emit AndroidGW::instance()->setBannerViewHeight(height);
}

extern "C" JNIEXPORT void JNICALL Java_com_derevenetz_oleg_magicphotos_stdalone_MagicActivity_imageSelected(JNIEnv *jni_env, jclass, jstring j_image_file, jint image_orientation)
{
    const char* str        = jni_env->GetStringUTFChars(j_image_file, nullptr);
    QString     image_file = str;

    jni_env->ReleaseStringUTFChars(j_image_file, str);

    emit AndroidGW::instance()->processImageSelection(image_file, image_orientation);
}

extern "C" JNIEXPORT void JNICALL Java_com_derevenetz_oleg_magicphotos_stdalone_MagicActivity_imageSelectionCancelled(JNIEnv *)
{
    emit AndroidGW::instance()->processImageSelectionCancel();
}

extern "C" JNIEXPORT void JNICALL Java_com_derevenetz_oleg_magicphotos_stdalone_MagicActivity_imageSelectionFailed(JNIEnv *)
{
    emit AndroidGW::instance()->processImageSelectionFailure();
}
