APP_NAME = MagicPhotos

CONFIG += qt warn_on cascades10

INCLUDEPATH += ../src ${QNX_TARGET}/usr/include/bb/cascades/pickers
DEPENDPATH += ../src ${QNX_TARGET}/usr/include/bb/cascades/pickers
LIBS += -lbbcascadespickers -lbb -lbbsystem

# Uncomment this to enable debugging output to console
DEFINES += DEBUG

include(config.pri)
