#include <QtAndroidExtras/QAndroidJniObject>

#include "androidgw.h"

AndroidGW *AndroidGW::Instance = NULL;

AndroidGW::AndroidGW(QObject *parent) : QObject(parent)
{
    Instance = this;
}

AndroidGW *AndroidGW::instance()
{
    return Instance;
}

static void bannerViewHeightUpdated(JNIEnv *, jclass, jint height)
{
    emit AndroidGW::instance()->setBannerViewHeight(height);
}

static void imageSelected(JNIEnv *jni_env, jclass, jstring j_image_file, jint image_orientation)
{
    const char* str        = jni_env->GetStringUTFChars(j_image_file, NULL);
    QString     image_file = str;

    jni_env->ReleaseStringUTFChars(j_image_file, str);

    emit AndroidGW::instance()->processImageSelection(image_file, image_orientation);
}

static void imageSelectionCancelled(JNIEnv *)
{
    emit AndroidGW::instance()->processImageSelectionCancel();
}

static void imageSelectionFailed(JNIEnv *)
{
    emit AndroidGW::instance()->processImageSelectionFailure();
}

static JNINativeMethod methods[] = {
    { "bannerViewHeightUpdated", "(I)V",                   (void *)bannerViewHeightUpdated },
    { "imageSelected",           "(Ljava/lang/String;I)V", (void *)imageSelected },
    { "imageSelectionCancelled", "()V",                    (void *)imageSelectionCancelled },
    { "imageSelectionFailed",    "()V",                    (void *)imageSelectionFailed }
};
static int methods_count = 4;

jint JNICALL JNI_OnLoad(JavaVM *vm, void *)
{
    JNIEnv *env;

    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_4) == JNI_OK) {
        bool   success = true;
        jclass clazz   = env->FindClass("com/derevenetz/oleg/magicphotos/MagicActivity");

        if (clazz != NULL) {
            success = false;

            if (env->RegisterNatives(clazz, methods, methods_count) >= 0) {
                success = true;
            }
        }

        if (success) {
            return JNI_VERSION_1_4;
        } else {
            return JNI_FALSE;
        }
    } else {
        return JNI_FALSE;
    }
}
