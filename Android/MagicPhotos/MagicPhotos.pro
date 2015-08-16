TEMPLATE = app

QT += gui widgets qml quick androidextras

SOURCES += main.cpp \
    appsettings.cpp \
    androidgw.cpp \
    helper.cpp \
    brushpreviewgenerator.cpp \
    decolorizeeditor.cpp \
    sketcheditor.cpp \
    cartooneditor.cpp \
    blureditor.cpp \
    pixelateeditor.cpp \
    recoloreditor.cpp \
    retoucheditor.cpp

HEADERS += \
    appsettings.h \
    androidgw.h \
    helper.h \
    brushpreviewgenerator.h \
    decolorizeeditor.h \
    sketcheditor.h \
    cartooneditor.h \
    blureditor.h \
    pixelateeditor.h \
    recoloreditor.h \
    retoucheditor.h

RESOURCES += \
    qml.qrc \
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
    android-source/src/com/derevenetz/oleg/magicphotos/MagicActivity.java \
    android-source/src/com/android/vending/billing/IInAppBillingService.aidl

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android-source

# Default rules for deployment.
include(deployment.pri)
