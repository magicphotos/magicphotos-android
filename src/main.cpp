#include <QtCore/QString>
#include <QtCore/QLocale>
#include <QtCore/QTranslator>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtQuickControls2/QQuickStyle>
#include <QtAndroidExtras/QtAndroid>

#include "admobhelper.h"
#include "androidgw.h"
#include "appsettings.h"
#include "blureditor.h"
#include "brushpreviewgenerator.h"
#include "cartooneditor.h"
#include "decolorizeeditor.h"
#include "gplayhelper.h"
#include "helper.h"
#include "mediastorehelper.h"
#include "pixelateeditor.h"
#include "recoloreditor.h"
#include "retoucheditor.h"
#include "sketcheditor.h"
#include "uihelper.h"

int main(int argc, char *argv[])
{
    QTranslator     translator;
    QGuiApplication app(argc, argv);

    if (translator.load(QStringLiteral(":/tr/magicphotos_%1").arg(QLocale::system().name()))) {
        QGuiApplication::installTranslator(&translator);
    }

    QObject::connect(&AndroidGW::GetInstance(), &AndroidGW::deviceConfigurationUpdated, &UIHelper::GetInstance(),    &UIHelper::handleDeviceConfigurationUpdate);
    QObject::connect(&AndroidGW::GetInstance(), &AndroidGW::bannerViewHeightUpdated,    &AdMobHelper::GetInstance(), &AdMobHelper::setBannerViewHeight);
    QObject::connect(&AndroidGW::GetInstance(), &AndroidGW::imageSelected,              &UIHelper::GetInstance(),    &UIHelper::handleImageSelection);
    QObject::connect(&AndroidGW::GetInstance(), &AndroidGW::imageSelectionCancelled,    &UIHelper::GetInstance(),    &UIHelper::handleImageSelectionCancel);
    QObject::connect(&AndroidGW::GetInstance(), &AndroidGW::imageSelectionFailed,       &UIHelper::GetInstance(),    &UIHelper::handleImageSelectionFailure);

    qmlRegisterUncreatableType<Editor>      ("ImageEditor", 1, 0, "Editor",       QStringLiteral("Could not create an object of type Editor"));
    qmlRegisterUncreatableType<EffectEditor>("ImageEditor", 1, 0, "EffectEditor", QStringLiteral("Could not create an object of type EffectEditor"));

    qmlRegisterType<BlurEditor>          ("ImageEditor", 1, 0, "BlurEditor");
    qmlRegisterType<BlurPreviewGenerator>("ImageEditor", 1, 0, "BlurPreviewGenerator");

    qmlRegisterType<BrushPreviewGenerator>("ImageEditor", 1, 0, "BrushPreviewGenerator");

    qmlRegisterType<CartoonEditor>          ("ImageEditor", 1, 0, "CartoonEditor");
    qmlRegisterType<CartoonPreviewGenerator>("ImageEditor", 1, 0, "CartoonPreviewGenerator");

    qmlRegisterType<DecolorizeEditor>("ImageEditor", 1, 0, "DecolorizeEditor");

    qmlRegisterType<Helper>("ImageEditor", 1, 0, "Helper");

    qmlRegisterType<PixelateEditor>          ("ImageEditor", 1, 0, "PixelateEditor");
    qmlRegisterType<PixelatePreviewGenerator>("ImageEditor", 1, 0, "PixelatePreviewGenerator");

    qmlRegisterType<RecolorEditor>("ImageEditor", 1, 0, "RecolorEditor");

    qmlRegisterType<RetouchEditor>("ImageEditor", 1, 0, "RetouchEditor");

    qmlRegisterType<SketchEditor>          ("ImageEditor", 1, 0, "SketchEditor");
    qmlRegisterType<SketchPreviewGenerator>("ImageEditor", 1, 0, "SketchPreviewGenerator");

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty(QStringLiteral("AdMobHelper"), &AdMobHelper::GetInstance());
    engine.rootContext()->setContextProperty(QStringLiteral("AppSettings"), &AppSettings::GetInstance());
    engine.rootContext()->setContextProperty(QStringLiteral("GPlayHelper"), &GPlayHelper::GetInstance());
    engine.rootContext()->setContextProperty(QStringLiteral("MediaStoreHelper"), &MediaStoreHelper::GetInstance());
    engine.rootContext()->setContextProperty(QStringLiteral("UIHelper"), &UIHelper::GetInstance());

    QQuickStyle::setStyle(QStringLiteral("Material"));

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    QtAndroid::hideSplashScreen();

    if (engine.rootObjects().isEmpty()) {
        return -1;
    } else {
        return QGuiApplication::exec();
    }
}
