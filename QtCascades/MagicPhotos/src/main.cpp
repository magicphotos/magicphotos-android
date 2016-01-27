#include <QtCore/QtGlobal>
#include <QtCore/QMetaType>
#include <QtCore/QLocale>
#include <QtCore/QTranslator>
#include <Qt/qdeclarativedebug.h>

#include <bb/cascades/Application>

#include "MagicPhotos.hpp"

using namespace bb::cascades;

void consoleMessageOutput(QtMsgType type, const char *msg)
{
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s\n", msg);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s\n", msg);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s\n", msg);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s\n", msg);
        abort();
        break;
    }
}

Q_DECL_EXPORT int main(int argc, char **argv)
{
#ifdef DEBUG_CONSOLE
    qInstallMsgHandler(consoleMessageOutput);
#endif

    Application app(argc, argv);

    QTranslator translator;
    QString     locale_string = QLocale().name();
    QString     filename      = QString("MagicPhotos_%1").arg(locale_string);

    if (translator.load(filename, "app/native/qm")) {
        app.installTranslator(&translator);
    }

    new MagicPhotos(&app);

    return Application::exec();
}
