import QtQuick 2.2
import QtQuick.Controls 1.1

import "Util.js" as UtilScript

ApplicationWindow {
    id:      mainWindow
    visible: true

    Component.onCompleted: {
        AppSettings.defaultBrushSize = UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 16);

        mainStackView.push(modeSelectionPage);
    }

    Rectangle {
        anchors.fill: parent
        color:        "black"

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

                if (depth === 1) {
                    modeSelectionPage.playModeChangeSuggestionAnimation();
                    modeSelectionPage.showPromoPopup();
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
}
