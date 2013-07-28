TARGET = MagicPhotos
VERSION = 2.0.0

TEMPLATE = app
QT += core gui declarative
CONFIG += qt-components mobility
MOBILITY += gallery

SOURCES += main.cpp \
    helper.cpp \
    decolorizeeditor.cpp
HEADERS += \
    helper.h \
    decolorizeeditor.h
OTHER_FILES += \
    icon.png \
    icon.svg

symbian: {
    TARGET.UID3 = 0xE29BCA98
    #TARGET.UID3 = 0x2004771D
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0x8000000

    ICON = icon.svg

    # SIS header: name, uid, version
    packageheader = "$${LITERAL_HASH}{\"MagicPhotos\"}, (0xE29BCA98), 2, 0, 0, TYPE=SA"
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
