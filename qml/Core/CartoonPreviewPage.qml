import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.3
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.12
import ImageEditor 1.0

import "../Util.js" as UtilScript

Page {
    id: cartoonPreviewPage

    header: Pane {
        Material.background: Material.Green

        Label {
            anchors.centerIn: parent
            text:             qsTr("Cartoon")
            font.pointSize:   24
        }
    }

    footer: ToolBar {
        RowLayout {
            anchors.fill: parent

            ToolButton {
                id:               applyToolButton
                implicitWidth:    UtilScript.pt(48)
                implicitHeight:   UtilScript.pt(48)
                enabled:          false
                Layout.alignment: Qt.AlignHCenter

                contentItem: Image {
                    source:   "qrc:/resources/images/tool_apply.png"
                    fillMode: Image.PreserveAspectFit
                }

                onClicked: {
                    var component = Qt.createComponent("CartoonPage.qml");

                    if (component.status === Component.Ready) {
                        mainStackView.push(component, {imageOrientation: imageOrientation, gaussianRadius: gaussianRadiusSlider.value, cartoonThreshold: thresholdSlider.value, imageFile: cartoonPreviewPage.imageFile});
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
            cartoonPreviewGenerator.radius    = gaussianRadiusSlider.value;
            cartoonPreviewGenerator.threshold = thresholdSlider.value;

            cartoonPreviewGenerator.openImage(imageFile, imageOrientation);
        }
    }

    onImageFileChanged: {
        if (imageOrientation !== -1 && imageFile !== "") {
            cartoonPreviewGenerator.radius    = gaussianRadiusSlider.value;
            cartoonPreviewGenerator.threshold = thresholdSlider.value;

            cartoonPreviewGenerator.openImage(imageFile, imageOrientation);
        }
    }

    ColumnLayout {
        anchors.fill:    parent
        anchors.margins: UtilScript.pt(16)
        spacing:         UtilScript.pt(16)

        Rectangle {
            color:             "transparent"
            Layout.fillWidth:  true
            Layout.fillHeight: true

            CartoonPreviewGenerator {
                id:           cartoonPreviewGenerator
                anchors.fill: parent

                property int waitRectangleUsageCounter: 0

                onImageOpened: {
                    gaussianRadiusSlider.enabled = true;
                    thresholdSlider.enabled      = true;
                    applyToolButton.enabled      = true;
                }

                onImageOpenFailed: {
                    gaussianRadiusSlider.enabled = false;
                    thresholdSlider.enabled      = false;
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
            from:             0
            to:               10
            value:            5
            stepSize:         1.0
            Layout.fillWidth: true

            onPressedChanged: {
                if (!pressed) {
                    cartoonPreviewGenerator.radius = value;
                }
            }
        }

        Slider {
            id:               thresholdSlider
            enabled:          false
            from:             32
            to:               128
            value:            80
            stepSize:         8.0
            Layout.fillWidth: true

            onPressedChanged: {
                if (!pressed) {
                    cartoonPreviewGenerator.threshold = value;
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
