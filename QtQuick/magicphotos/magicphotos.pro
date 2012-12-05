TARGET = MagicPhotos
VERSION = 1.0.8

TEMPLATE = app
QT += core gui declarative
CONFIG += qt-components mobility
MOBILITY += gallery

SOURCES += main.cpp \
    paintitem.cpp
HEADERS += \
    paintitem.h
RESOURCES += \
    magicphotos.qrc
OTHER_FILES += \
    doc/help.html \
    images/examples/example.png \
    images/splash.png \
    images/busy_indicator.png \
    images/exit.png \
    images/help.png \
    images/ok.png \
    images/cancel.png \
    images/open.png \
    images/undo.png \
    images/effect_grayscale.png \
    images/effect_sketch.png \
    images/effect_blur.png \
    images/effect_hue.png \
    images/mode_scroll.png \
    images/mode_original.png \
    images/mode_effect_grayscale.png \
    images/mode_effect_sketch.png \
    images/mode_effect_blur.png \
    images/mode_effect_hue.png \
    images/hue_picker.png \
    images/scale.png \
    images/save.png \
    images/parent.png \
    images/dialog_warning.png \
    images/dialog_error.png \
    images/dialog_question.png \
    images/fs_folder.png \
    images/fs_file_image.png \
    icon.png \
    icon.svg

symbian: {
    #TARGET.UID3 = 0xE29BCA98
    TARGET.UID3 = 0x2004771D
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0x8000000

    ICON = icon.svg

    # SIS header: name, uid, version
    packageheader = "$${LITERAL_HASH}{\"MagicPhotos\"}, (0x2004771D), 1, 0, 8, TYPE=SA"
    # Vendor info: localised and non-localised vendor names
    vendorinfo = "%{\"Oleg Derevenetz\"}" ":\"Oleg Derevenetz\""

    my_deployment.pkg_prerules = packageheader vendorinfo
    DEPLOYMENT += my_deployment

    # SIS installer header: uid
    DEPLOYMENT.installer_header = 0x2002CCCF
}

contains(MEEGO_EDITION,harmattan) {
    DEFINES += MEEGO_TARGET

    target.path = /opt/magicphotos/bin

    launchericon.files = magicphotos.svg
    launchericon.path = /usr/share/themes/base/meegotouch/icons/

    INSTALLS += target launchericon
}

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

# QML deployment
folder_qml.source = qml/magicphotos
folder_qml.target = qml
DEPLOYMENTFOLDERS = folder_qml

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()

contains(MEEGO_EDITION,harmattan) {
    desktopfile.files = magicphotos.desktop
    desktopfile.path = /usr/share/applications
    INSTALLS += desktopfile
}
