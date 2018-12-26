QT += quick quickcontrols2
CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS

# Uncomment to build full version
# DEFINES += FULL_VERSION

SOURCES += src/main.cpp \
    src/appsettings.cpp \
    src/androidgw.cpp \
    src/admobhelper.cpp \
    src/uihelper.cpp \
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
    src/admobhelper.h \
    src/uihelper.h \
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

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

android {
    QT += androidextras

    OTHER_FILES += \
        android/source/AndroidManifest.xml \
        android/source/build.gradle \
        android/source/gradlew \
        android/source/gradlew.bat \
        android/source/gradle/wrapper/gradle-wrapper.jar \
        android/source/gradle/wrapper/gradle-wrapper.properties \
        android/source/res/drawable/splash_qt.xml \
        android/source/res/drawable/splash_theme.xml \
        android/source/res/drawable-hdpi/ic_splash_qt.png \
        android/source/res/drawable-hdpi/ic_splash_theme.png \
        android/source/res/drawable-mdpi/ic_splash_qt.png \
        android/source/res/drawable-mdpi/ic_splash_theme.png \
        android/source/res/drawable-xhdpi/ic_splash_qt.png \
        android/source/res/drawable-xhdpi/ic_splash_theme.png \
        android/source/res/drawable-xxhdpi/ic_splash_qt.png \
        android/source/res/drawable-xxhdpi/ic_splash_theme.png \
        android/source/res/drawable-xxxhdpi/ic_splash_qt.png \
        android/source/res/drawable-xxxhdpi/ic_splash_theme.png \
        android/source/res/mipmap-hdpi/ic_launcher.png \
        android/source/res/mipmap-mdpi/ic_launcher.png \
        android/source/res/mipmap-xhdpi/ic_launcher.png \
        android/source/res/mipmap-xxhdpi/ic_launcher.png \
        android/source/res/mipmap-xxxhdpi/ic_launcher.png \
        android/source/res/values/colors.xml \
        android/source/res/values/libs.xml \
        android/source/res/values/strings.xml \
        android/source/res/values/themes.xml \
        android/source/res/values-ru/strings.xml \
        android/source/res/values-de/strings.xml \
        android/source/res/values-fr/strings.xml \
        android/source/res/xml/provider_paths.xml \
        android/source/src/com/derevenetz/oleg/magicphotos/stdalone/MagicActivity.java

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android/source
}

# Default rules for deployment.
include(deployment.pri)
