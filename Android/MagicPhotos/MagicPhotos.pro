TEMPLATE = app

QT += gui widgets qml quick androidextras

SOURCES += main.cpp \
    appsettings.cpp \
    decolorizeeditor.cpp \
    helper.cpp \
    sketcheditor.cpp \
    cartooneditor.cpp \
    blureditor.cpp \
    pixelateeditor.cpp \
    recoloreditor.cpp \
    retoucheditor.cpp \
    androidgw.cpp

HEADERS += \
    appsettings.h \
    decolorizeeditor.h \
    helper.h \
    sketcheditor.h \
    cartooneditor.h \
    blureditor.h \
    pixelateeditor.h \
    recoloreditor.h \
    retoucheditor.h \
    androidgw.h

RESOURCES += qml.qrc

OTHER_FILES += \
    android-source/AndroidManifest.xml \
    android-source/res/values/strings.xml \
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
