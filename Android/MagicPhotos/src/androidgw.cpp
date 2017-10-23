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
    QAndroidJniObject str_object = QAndroidJniObject::callStaticObjectMethod<jstring>("com/derevenetz/oleg/magicphotos/MagicActivity",
                                                                                      "getSaveDirectory");

    return str_object.toString();
}

int AndroidGW::getScreenDPI()
{
    return QAndroidJniObject::callStaticMethod<jint>("com/derevenetz/oleg/magicphotos/MagicActivity",
                                                     "getScreenDPI");
}

void AndroidGW::showGallery()
{
    QAndroidJniObject::callStaticMethod<void>("com/derevenetz/oleg/magicphotos/MagicActivity",
                                              "showGallery");
}

void AndroidGW::refreshGallery(const QString &image_file)
{
    QAndroidJniObject j_image_file = QAndroidJniObject::fromString(image_file);

    QAndroidJniObject::callStaticMethod<void>("com/derevenetz/oleg/magicphotos/MagicActivity",
                                              "refreshGallery", "(Ljava/lang/String;)V", j_image_file.object<jstring>());
}

void AndroidGW::shareImage(const QString &image_file)
{
    QAndroidJniObject j_image_file = QAndroidJniObject::fromString(image_file);

    QAndroidJniObject::callStaticMethod<void>("com/derevenetz/oleg/magicphotos/MagicActivity",
                                              "shareImage", "(Ljava/lang/String;)V", j_image_file.object<jstring>());
}

void AndroidGW::showAdView()
{
    QAndroidJniObject::callStaticMethod<void>("com/derevenetz/oleg/magicphotos/MagicActivity",
                                              "showAdView");
}

void AndroidGW::hideAdView()
{
    QAndroidJniObject::callStaticMethod<void>("com/derevenetz/oleg/magicphotos/MagicActivity",
                                              "hideAdView");
}

void AndroidGW::createInterstitialAd()
{
    QAndroidJniObject::callStaticMethod<void>("com/derevenetz/oleg/magicphotos/MagicActivity",
                                              "createInterstitialAd");
}

void AndroidGW::showInterstitialAd()
{
    QAndroidJniObject::callStaticMethod<void>("com/derevenetz/oleg/magicphotos/MagicActivity",
                                              "showInterstitialAd");
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
