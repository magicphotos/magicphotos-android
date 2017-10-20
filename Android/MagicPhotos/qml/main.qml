import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

import "Core"

import "Util.js" as UtilScript

ApplicationWindow {
    id:      mainWindow
    visible: true

    Material.theme:   Material.System
    Material.primary: Material.Teal

    Component.onCompleted: {
        AppSettings.defaultBrushSize = UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 16);

        mainStackView.push(modeSelectionPage);
    }

    StackView {
        id:           mainStackView
        anchors.fill: parent

        onCurrentItemChanged: {
            for (var i = 0; i < depth; i++) {
                var item = get(i, false);

                if (item !== null) {
                    item.focus = false;
                }
            }

            if (depth > 0) {
                get(depth - 1).forceActiveFocus();
            }
        }
    }

    ModeSelectionPage {
        id: modeSelectionPage
    }

    MouseArea {
        anchors.fill: parent
        z:            20
        enabled:      mainStackView.busy
    }
}
