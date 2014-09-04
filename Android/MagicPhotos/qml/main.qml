import QtQuick 2.2
import QtQuick.Controls 1.1

ApplicationWindow {
    id:      mainWindow
    visible: true

    Component.onCompleted: {
        mainStackView.push(modeSelectionPage);
    }

    Rectangle {
        anchors.fill: parent
        color:        "black"

        StackView  {
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
                    get(depth - 1).focus = true;
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
