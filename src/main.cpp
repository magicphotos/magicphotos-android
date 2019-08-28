#include <QtCore/QString>
#include <QtCore/QLocale>
#include <QtCore/QTranslator>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtQuickControls2/QQuickStyle>
#include <QtAndroidExtras/QtAndroid>

#include "appsettings.h"
#include "androidgw.h"
#include "admobhelper.h"
#include "uihelper.h"
#include "helper.h"
#include "brushpreviewgenerator.h"
#include "decolorizeeditor.h"
#include "sketcheditor.h"
#include "cartooneditor.h"
#include "blureditor.h"
#include "pixelateeditor.h"
#include "recoloreditor.h"
#include "retoucheditor.h"

int main(int argc, char *argv[])
{
    QTranslator     translator;
    QGuiApplication app(argc, argv);

    if (translator.load(QString(":/tr/magicphotos_%1").arg(QLocale::system().name()))) {
        QGuiApplication::installTranslator(&translator);
    }

    QObject::connect(&AndroidGW::GetInstance(), &AndroidGW::bannerViewHeightChanged, &AdMobHelper::GetInstance(), &AdMobHelper::setBannerViewHeight);
    QObject::connect(&AndroidGW::GetInstance(), &AndroidGW::imageSelected,           &UIHelper::GetInstance(),    &UIHelper::handleImageSelection);
    QObject::connect(&AndroidGW::GetInstance(), &AndroidGW::imageSelectionCancelled, &UIHelper::GetInstance(),    &UIHelper::handleImageSelectionCancel);
    QObject::connect(&AndroidGW::GetInstance(), &AndroidGW::imageSelectionFailed,    &UIHelper::GetInstance(),    &UIHelper::handleImageSelectionFailure);

    qmlRegisterType<Helper>("ImageEditor", 1, 0, "Helper");
    qmlRegisterType<BrushPreviewGenerator>("ImageEditor", 1, 0, "BrushPreviewGenerator");
    qmlRegisterType<DecolorizeEditor>("ImageEditor", 1, 0, "DecolorizeEditor");

    qmlRegisterType<SketchEditor>("ImageEditor", 1, 0, "SketchEditor");
    qmlRegisterType<SketchPreviewGenerator>("ImageEditor", 1, 0, "SketchPreviewGenerator");

    qmlRegisterType<CartoonEditor>("ImageEditor", 1, 0, "CartoonEditor");
    qmlRegisterType<CartoonPreviewGenerator>("ImageEditor", 1, 0, "CartoonPreviewGenerator");

    qmlRegisterType<BlurEditor>("ImageEditor", 1, 0, "BlurEditor");
    qmlRegisterType<BlurPreviewGenerator>("ImageEditor", 1, 0, "BlurPreviewGenerator");

    qmlRegisterType<PixelateEditor>("ImageEditor", 1, 0, "PixelateEditor");
    qmlRegisterType<PixelatePreviewGenerator>("ImageEditor", 1, 0, "PixelatePreviewGenerator");

    qmlRegisterType<RecolorEditor>("ImageEditor", 1, 0, "RecolorEditor");

    qmlRegisterType<RetouchEditor>("ImageEditor", 1, 0, "RetouchEditor");

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty(QStringLiteral("AppSettings"), &AppSettings::GetInstance());
    engine.rootContext()->setContextProperty(QStringLiteral("AdMobHelper"), &AdMobHelper::GetInstance());
    engine.rootContext()->setContextProperty(QStringLiteral("UIHelper"), &UIHelper::GetInstance());

    QQuickStyle::setStyle("Material");

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    QtAndroid::hideSplashScreen();

    if (engine.rootObjects().isEmpty()) {
        return -1;
    } else {
        return QGuiApplication::exec();
    }
}
