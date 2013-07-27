#include <QApplication>

#include "qmlapplicationviewer.h"
#include "decolorizeeditor.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
#ifndef MEEGO_TARGET
    QmlApplicationViewer splash;
#endif
    QmlApplicationViewer viewer;

    qmlRegisterType<DecolorizeEditor>("ImageEditor", 1, 0, "DecolorizeEditor");

#ifndef MEEGO_TARGET
    splash.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    splash.setMainQmlFile(QLatin1String("qml/magicphotos/Symbian/splash.qml"));
    splash.showFullScreen();

    QCoreApplication::processEvents();
#endif

    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
#ifdef MEEGO_TARGET
    viewer.setMainQmlFile(QLatin1String("qml/magicphotos/Meego/main.qml"));
#else
    viewer.setMainQmlFile(QLatin1String("qml/magicphotos/Symbian/main.qml"));
#endif
    viewer.showFullScreen();

#ifndef MEEGO_TARGET
    splash.close();
#endif

    return app.exec();
}
