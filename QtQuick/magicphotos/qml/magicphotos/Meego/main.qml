import QtQuick 1.1
import com.nokia.meego 1.0

import "Core"

import "Settings.js" as SettingsScript

Window {
    id: mainWindow

    Component.onCompleted: {
        mainPageStack.push(modeSelectionPage);
    }

    Rectangle {
        anchors.fill: parent
        color:        "black"

        PageStack {
            id:           mainPageStack
            anchors.fill: parent
        }

        ModeSelectionPage {
            id: modeSelectionPage
        }

        MouseArea {
            anchors.fill: parent
            z:            20
            enabled:      mainPageStack.busy
        }
    }
}
