#include <QtAndroidExtras/QtAndroid>
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

QString AndroidGW::getSaveDirectory()
{
    QAndroidJniObject str_object = QtAndroid::androidActivity().callObjectMethod<jstring>("getSaveDirectory");

    return str_object.toString();
}

int AndroidGW::getScreenDPI()
{
    return QtAndroid::androidActivity().callMethod<jint>("getScreenDPI");
}

void AndroidGW::showGallery()
{
    QtAndroid::androidActivity().callMethod<void>("showGallery");
}

void AndroidGW::refreshGallery(QString image_file)
{
    QAndroidJniObject j_image_file = QAndroidJniObject::fromString(image_file);

    QtAndroid::androidActivity().callMethod<void>("refreshGallery", "(Ljava/lang/String;)V", j_image_file.object<jstring>());
}

void AndroidGW::shareImage(QString image_file)
{
    QAndroidJniObject j_image_file = QAndroidJniObject::fromString(image_file);

    QtAndroid::androidActivity().callMethod<void>("shareImage", "(Ljava/lang/String;)V", j_image_file.object<jstring>());
}

void AndroidGW::showAdView()
{
    QtAndroid::androidActivity().callMethod<void>("showAdView");
}

void AndroidGW::hideAdView()
{
    QtAndroid::androidActivity().callMethod<void>("hideAdView");
}

void AndroidGW::createInterstitialAd()
{
    QtAndroid::androidActivity().callMethod<void>("createInterstitialAd");
}

void AndroidGW::showInterstitialAd()
{
    QtAndroid::androidActivity().callMethod<void>("showInterstitialAd");
}

static void adViewHeightUpdated(JNIEnv *, jclass, jint adview_height)
{
    emit AndroidGW::instance()->adViewHeightUpdated(adview_height);
}

static void imageSelected(JNIEnv *jni_env, jclass, jstring j_image_file, jint image_orientation)
{
    const char* str        = jni_env->GetStringUTFChars(j_image_file, NULL);
    QString     image_file = str;

    jni_env->ReleaseStringUTFChars(j_image_file, str);

    emit AndroidGW::instance()->imageSelected(image_file, image_orientation);
}

static void imageSelectionCancelled(JNIEnv *)
{
    emit AndroidGW::instance()->imageSelectionCancelled();
}

static void imageSelectionFailed(JNIEnv *)
{
    emit AndroidGW::instance()->imageSelectionFailed();
}

static JNINativeMethod methods[] = {
    { "adViewHeightUpdated",     "(I)V",                   (void *)adViewHeightUpdated },
    { "imageSelected",           "(Ljava/lang/String;I)V", (void *)imageSelected },
    { "imageSelectionCancelled", "()V",                    (void *)imageSelectionCancelled },
    { "imageSelectionFailed",    "()V",                    (void *)imageSelectionFailed }
};

jint JNICALL JNI_OnLoad(JavaVM *vm, void *)
{
    JNIEnv *env;

    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_4) == JNI_OK) {
        jclass clazz = env->FindClass("com/derevenetz/oleg/magicphotos/MagicActivity");

        if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) >= 0) {
            return JNI_VERSION_1_4;
        } else {
            return JNI_FALSE;
        }
    } else {
        return JNI_FALSE;
    }
}
