import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3
import ImageEditor 1.0

import "../Util.js" as UtilScript

Page {
    id: blurPreviewPage

    header: Pane {
        Material.background: Material.Green

        Label {
            anchors.centerIn: parent
            text:             qsTr("Blur")
            font.pointSize:   24
        }
    }

    footer: ToolBar {
        RowLayout {
            anchors.fill: parent

            ToolButton {
                id:               applyToolButton
                implicitWidth:    UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                implicitHeight:   UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                enabled:          false
                Layout.alignment: Qt.AlignHCenter

                contentItem: Image {
                    source:   "qrc:/resources/images/tool_apply.png"
                    fillMode: Image.PreserveAspectFit
                }

                onClicked: {
                    var component = Qt.createComponent("BlurPage.qml");

                    if (component.status === Component.Ready) {
                        mainStackView.push(component, {imageOrientation: imageOrientation, gaussianRadius: gaussianRadiusSlider.value, imageFile: blurPreviewPage.imageFile});
                    } else {
                        console.log(component.errorString());
                    }
                }
            }
        }
    }

    property int    imageOrientation: -1

    property string imageFile:        ""

    Keys.onReleased: {
        if (event.key === Qt.Key_Back) {
            mainStackView.pop();

            event.accepted = true;
        }
    }

    onImageOrientationChanged: {
        if (imageOrientation !== -1 && imageFile !== "") {
            blurPreviewGenerator.radius = gaussianRadiusSlider.value;

            blurPreviewGenerator.openImage(imageFile, imageOrientation);
        }
    }

    onImageFileChanged: {
        if (imageOrientation !== -1 && imageFile !== "") {
            blurPreviewGenerator.radius = gaussianRadiusSlider.value;

            blurPreviewGenerator.openImage(imageFile, imageOrientation);
        }
    }

    ColumnLayout {
        anchors.fill:    parent
        anchors.margins: UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 16)
        spacing:         UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 16)

        Rectangle {
            color:             "transparent"
            Layout.fillWidth:  true
            Layout.fillHeight: true

            BlurPreviewGenerator {
                id:           blurPreviewGenerator
                anchors.fill: parent

                property int waitRectangleUsageCounter: 0

                onImageOpened: {
                    gaussianRadiusSlider.enabled = true;
                    applyToolButton.enabled      = true;
                }

                onImageOpenFailed: {
                    gaussianRadiusSlider.enabled = false;
                    applyToolButton.enabled      = false;

                    imageOpenFailedMessageDialog.open();
                }

                onGenerationStarted: {
                    waitRectangleUsageCounter = waitRectangleUsageCounter + 1;

                    if (waitRectangleUsageCounter === 1) {
                        waitRectangle.visible = true;
                    }
                }

                onGenerationFinished: {
                    if (waitRectangleUsageCounter === 1) {
                        waitRectangle.visible = false;
                    }

                    if (waitRectangleUsageCounter > 0) {
                        waitRectangleUsageCounter = waitRectangleUsageCounter - 1;
                    }
                }
            }

            Rectangle {
                id:           waitRectangle
                anchors.fill: parent
                z:            10
                color:        "black"
                opacity:      0.75
                visible:      false

                BusyIndicator {
                    anchors.centerIn: parent
                    running:          parent.visible
                }

                MouseArea {
                    anchors.fill: parent
                }
            }
        }

        Slider {
            id:               gaussianRadiusSlider
            enabled:          false
            from:             4
            to:               18
            value:            11
            stepSize:         1.0
            Layout.fillWidth: true

            onPressedChanged: {
                if (!pressed) {
                    blurPreviewGenerator.radius = value;
                }
            }
        }
    }

    MessageDialog {
        id:              imageOpenFailedMessageDialog
        title:           qsTr("Error")
        icon:            StandardIcon.Critical
        text:            qsTr("Could not open image")
        standardButtons: StandardButton.Ok
    }
}
