import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.1
import ImageEditor 1.0

import "Util.js" as UtilScript

Item {
    id:    cartoonPreviewPage
    focus: true

    property int    imageOrientation: -1

    property string openImageFile:    ""

    Keys.onReleased: {
        if (event.key === Qt.Key_Back) {
            mainStackView.pop();

            event.accepted = true;
        }
    }

    onImageOrientationChanged: {
        if (imageOrientation !== -1 && openImageFile !== "") {
            cartoonPreviewGenerator.radius    = gaussianRadiusSlider.value;
            cartoonPreviewGenerator.threshold = thresholdSlider.value;

            cartoonPreviewGenerator.openImage(openImageFile, imageOrientation);
        }
    }

    onOpenImageFileChanged: {
        if (imageOrientation !== -1 && openImageFile !== "") {
            cartoonPreviewGenerator.radius    = gaussianRadiusSlider.value;
            cartoonPreviewGenerator.threshold = thresholdSlider.value;

            cartoonPreviewGenerator.openImage(openImageFile, imageOrientation);
        }
    }

    Rectangle {
        anchors.top:    parent.top
        anchors.bottom: gaussianRadiusSliderRectangle.top
        anchors.left:   parent.left
        anchors.right:  parent.right
        color:          "transparent"

        CartoonPreviewGenerator {
            id:           cartoonPreviewGenerator
            anchors.fill: parent

            property int waitRectangleUsageCounter: 0

            onImageOpened: {
                gaussianRadiusSlider.enabled = true;
                thresholdSlider.enabled      = true;
                applyButton.enabled          = true;
            }

            onImageOpenFailed: {
                gaussianRadiusSlider.enabled = false;
                thresholdSlider.enabled      = false;
                applyButton.enabled          = false;

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
            visible:      false
            color:        "black"
            opacity:      0.75

            MouseArea {
                anchors.fill: parent

                Image {
                    anchors.centerIn: parent
                    source:           "images/busy_indicator.png"
                }
            }
        }
    }

    Rectangle {
        id:             gaussianRadiusSliderRectangle
        anchors.bottom: thresholdSliderRectangle.top
        anchors.left:   parent.left
        anchors.right:  parent.right
        height:         gaussianRadiusSlider.height + UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 16)
        color:          "transparent"

        Slider {
            id:                     gaussianRadiusSlider
            anchors.verticalCenter: parent.verticalCenter
            anchors.left:           parent.left
            anchors.right:          parent.right
            enabled:                false
            minimumValue:           0
            maximumValue:           10
            value:                  5
            stepSize:               1.0

            onPressedChanged: {
                if (!pressed) {
                    cartoonPreviewGenerator.radius = value;
                }
            }
        }
    }

    Rectangle {
        id:             thresholdSliderRectangle
        anchors.bottom: applyButtonRectangle.top
        anchors.left:   parent.left
        anchors.right:  parent.right
        height:         thresholdSlider.height + UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 16)
        color:          "transparent"

        Slider {
            id:                     thresholdSlider
            anchors.verticalCenter: parent.verticalCenter
            anchors.left:           parent.left
            anchors.right:          parent.right
            enabled:                false
            minimumValue:           32
            maximumValue:           128
            value:                  80
            stepSize:               8.0

            onPressedChanged: {
                if (!pressed) {
                    cartoonPreviewGenerator.threshold = value;
                }
            }
        }
    }

    Rectangle {
        id:             applyButtonRectangle
        anchors.bottom: bottomToolBar.top
        anchors.left:   parent.left
        anchors.right:  parent.right
        height:         applyButton.height + UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 16)
        color:          "transparent"

        Button {
            id:               applyButton
            anchors.centerIn: parent
            enabled:          false
            text:             qsTr("Apply")

            onClicked: {
                var component = Qt.createComponent("CartoonPage.qml");

                if (component.status === Component.Ready) {
                    mainStackView.push({item: component.createObject(null), destroyOnPop: true, properties: {imageOrientation: imageOrientation, gaussianRadius: gaussianRadiusSlider.value, cartoonThreshold: thresholdSlider.value, openImageFile: openImageFile}});
                } else {
                    console.log(component.errorString());
                }
            }
        }
    }

    ToolBar {
        id:             bottomToolBar
        anchors.bottom: parent.bottom
        height:         UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
        z:              1

        RowLayout {
            anchors.fill: parent

            ToolButton {
                anchors.left: parent.left
                width:        UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                height:       UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)

                style: ButtonStyle {
                    background: Rectangle {
                        implicitWidth:  control.width
                        implicitHeight: control.height
                        color:          "transparent"

                        Image {
                            anchors.fill:    parent
                            anchors.margins: UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 4)
                            source:          "images/back.png"
                            fillMode:        Image.PreserveAspectFit
                        }
                    }
                }

                onClicked: {
                    mainStackView.pop();
                }
            }

            ToolButton {
                anchors.centerIn: parent
                width:            UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                height:           UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)

                style: ButtonStyle {
                    background: Rectangle {
                        implicitWidth:  control.width
                        implicitHeight: control.height
                        color:          "transparent"

                        Image {
                            anchors.fill:    parent
                            anchors.margins: UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 4)
                            source:          "images/help.png"
                            fillMode:        Image.PreserveAspectFit
                        }
                    }
                }

                onClicked: {
                    Qt.openUrlExternally(qsTr("http://m.youtube.com/"));
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
