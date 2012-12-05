import QtQuick 1.0
import com.nokia.meego 1.0

import "Core"

import "Settings.js" as SettingsScript

Window {
    id: mainWindow

    Rectangle {
        id:           backgroundRectangle
        anchors.fill: parent
        color:        "black"

        PageStack {
            id:           mainPageStack
            anchors.fill: parent
        }

        PaintPage {
            id: paintPage
        }

        HelpPage {
            id: helpPage
        }

        FileOpenPage {
            id: fileOpenPage

            onFileSelected: {
                mainPageStack.replace(paintPage);

                paintPage.openImage(fileUrl);
            }

            onOpenCancelled: {
                mainPageStack.replace(paintPage);
            }
        }

        FileSavePage {
            id: fileSavePage

            onFileSelected: {
                mainPageStack.replace(paintPage);

                paintPage.saveImage(fileUrl);
            }

            onSaveCancelled: {
                mainPageStack.replace(paintPage);

                paintPage.saveCancelled();
            }
        }

        MouseArea {
            id:           screenLockMouseArea
            anchors.fill: parent
            z:            20
            enabled:      mainPageStack.busy
        }
    }

    Component.onCompleted: {
        mainPageStack.push(paintPage);

        if (SettingsScript.getSetting("FirstRun", "TRUE") === "TRUE") {
            paintPage.offerExampleImage();

            SettingsScript.setSetting("FirstRun", "FALSE");
        }
    }
}
