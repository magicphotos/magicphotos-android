APP_NAME = MagicPhotos

CONFIG += qt warn_on cascades10

LIBS += -lbbcascadespickers -lbbplatformbbm -lbb -lbbsystem -lexif

TRANSLATIONS = $${TARGET}_ru.ts \
               $${TARGET}_de.ts \
               $${TARGET}_fr.ts \
               $${TARGET}.ts

# Uncomment this to enable debugging output to console
#DEFINES += DEBUG_CONSOLE

# Uncomment this for trial version
#DEFINES += TRIAL_VERSION

include(config.pri)
