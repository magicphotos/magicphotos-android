QT += quick quickcontrols2 purchasing androidextras
CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS

# Uncomment to build full version
# DEFINES += FULL_VERSION

SOURCES += src/main.cpp \
    src/appsettings.cpp \
    src/androidgw.cpp \
    src/helper.cpp \
    src/brushpreviewgenerator.cpp \
    src/decolorizeeditor.cpp \
    src/sketcheditor.cpp \
    src/cartooneditor.cpp \
    src/blureditor.cpp \
    src/pixelateeditor.cpp \
    src/recoloreditor.cpp \
    src/retoucheditor.cpp

HEADERS += \
    src/appsettings.h \
    src/androidgw.h \
    src/helper.h \
    src/brushpreviewgenerator.h \
    src/decolorizeeditor.h \
    src/sketcheditor.h \
    src/cartooneditor.h \
    src/blureditor.h \
    src/pixelateeditor.h \
    src/recoloreditor.h \
    src/retoucheditor.h

RESOURCES += \
    qml.qrc \
    resources.qrc \
    translations.qrc

TRANSLATIONS += \
    translations/MagicPhotos_ru.ts \
    translations/MagicPhotos_de.ts \
    translations/MagicPhotos_fr.ts

OTHER_FILES += \
    android-source/AndroidManifest.xml \
    android-source/res/values/strings.xml \
    android-source/res/values-ru/strings.xml \
    android-source/res/values-de/strings.xml \
    android-source/res/values-fr/strings.xml \
    android-source/res/drawable-hdpi/ic_launcher.png \
    android-source/res/drawable-mdpi/ic_launcher.png \
    android-source/res/drawable-xhdpi/ic_launcher.png \
    android-source/res/drawable-xxhdpi/ic_launcher.png \
    android-source/src/com/derevenetz/oleg/magicphotos/MagicActivity.java

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android-source

# Default rules for deployment.
include(deployment.pri)
