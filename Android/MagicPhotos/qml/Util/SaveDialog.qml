import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.1

import "../Util.js" as UtilScript

Rectangle {
    id:           saveDialog
    anchors.fill: parent
    z:            15
    color:        "transparent"
    focus:        false
    visible:      false

    property string filePath: ""

    signal okPressed(string file_path, string file_name)
    signal cancelPressed()

    function show(file) {
        if (file !== "") {
            var pathNameRegexp = /^(.+)\/([^/]+)$/;
            var pathNameArr;

            if ((pathNameArr = pathNameRegexp.exec(file)) !== null) {
                filePath               = pathNameArr[1];
                fileNameTextField.text = pathNameArr[2];
            } else {
                filePath               = "";
                fileNameTextField.text = "";
            }
        }

        focus   = true;
        visible = true;
    }

    Keys.onReleased: {
        if (event.key === Qt.Key_Back) {
            focus   = false;
            visible = false;

            cancelPressed();

            event.accepted = true;
        }
    }

    Rectangle {
        anchors.fill: parent
        color:        "black"
        opacity:      0.5

        MouseArea {
            anchors.fill: parent
        }
    }

    Rectangle {
        anchors.centerIn: parent
        opacity:          1.0
        color:            "lightgray"
        width:            parent.width * 3 / 4
        height:           labelText.height + fileNameTextField.height + Math.max(okButton.height, cancelButton.height) + UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 32)

        Text {
            id:                 labelText
            anchors.top:        parent.top
            anchors.left:       parent.left
            anchors.leftMargin: UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 5)
            color:              "black"
            font.pointSize:     18
            text:               qsTr("File Name")
        }

        TextField {
            id:                     fileNameTextField
            anchors.verticalCenter: parent.verticalCenter
            anchors.left:           parent.left
            anchors.right:          parent.right
            anchors.leftMargin:     UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 5)
            anchors.rightMargin:    UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 5)
            inputMethodHints:       Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
        }

        Button {
            id:                   okButton
            anchors.bottom:       parent.bottom
            anchors.left:         parent.left
            anchors.leftMargin:   UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 5)
            anchors.bottomMargin: UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 5)
            width:                parent.width / 2 - UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 10)
            text:                 qsTr("OK")

            onClicked: {
                saveDialog.focus   = false;
                saveDialog.visible = false;

                saveDialog.okPressed(saveDialog.filePath, fileNameTextField.text);
            }
        }

        Button {
            id:                   cancelButton
            anchors.bottom:       parent.bottom
            anchors.right:        parent.right
            anchors.rightMargin:  UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 5)
            anchors.bottomMargin: UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 5)
            width:                parent.width / 2 - UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 10)
            text:                 qsTr("Cancel")

            onClicked: {
                saveDialog.focus   = false;
                saveDialog.visible = false;

                saveDialog.cancelPressed();
            }
        }
    }
}
