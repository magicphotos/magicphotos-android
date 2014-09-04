#include <QtWidgets/QApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>

#include "androidgw.h"
#include "decolorizeeditor.h"
#include "helper.h"
#include "sketcheditor.h"
#include "cartooneditor.h"
#include "blureditor.h"
#include "pixelateeditor.h"
#include "recoloreditor.h"
#include "retoucheditor.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qmlRegisterType<DecolorizeEditor>("ImageEditor", 1, 0, "DecolorizeEditor");
    qmlRegisterType<Helper>("ImageEditor", 1, 0, "Helper");

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

    engine.rootContext()->setContextProperty(QStringLiteral("AndroidGW"), new AndroidGW(&app));

    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    return app.exec();
}
