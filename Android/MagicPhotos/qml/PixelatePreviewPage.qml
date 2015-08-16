import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.1
import ImageEditor 1.0

import "Util.js" as UtilScript

Item {
    id: pixelatePreviewPage

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
            pixelatePreviewGenerator.pixDenom = pixDenomSlider.value;

            pixelatePreviewGenerator.openImage(imageFile, imageOrientation);
        }
    }

    onImageFileChanged: {
        if (imageOrientation !== -1 && imageFile !== "") {
            pixelatePreviewGenerator.pixDenom = pixDenomSlider.value;

            pixelatePreviewGenerator.openImage(imageFile, imageOrientation);
        }
    }

    Rectangle {
        anchors.top:    parent.top
        anchors.bottom: pixDenomSliderRectangle.top
        anchors.left:   parent.left
        anchors.right:  parent.right
        color:          "transparent"

        PixelatePreviewGenerator {
            id:           pixelatePreviewGenerator
            anchors.fill: parent

            property int waitRectangleUsageCounter: 0

            onImageOpened: {
                pixDenomSlider.enabled = true;
                applyButton.enabled    = true;
            }

            onImageOpenFailed: {
                pixDenomSlider.enabled = false;
                applyButton.enabled    = false;

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
                    width:            UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                    height:           UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                    source:           "images/busy_indicator.png"
                    fillMode:         Image.PreserveAspectFit
                }
            }
        }
    }

    Rectangle {
        id:             pixDenomSliderRectangle
        anchors.bottom: applyButtonRectangle.top
        anchors.left:   parent.left
        anchors.right:  parent.right
        height:         pixDenomSlider.height + UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 16)
        color:          "transparent"

        Slider {
            id:                     pixDenomSlider
            anchors.verticalCenter: parent.verticalCenter
            anchors.left:           parent.left
            anchors.right:          parent.right
            enabled:                false
            minimumValue:           32
            maximumValue:           192
            value:                  112
            stepSize:               8.0

            onPressedChanged: {
                if (!pressed) {
                    pixelatePreviewGenerator.pixDenom = value;
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
                var component = Qt.createComponent("PixelatePage.qml");

                if (component.status === Component.Ready) {
                    mainStackView.push({item: component.createObject(null), destroyOnPop: true, properties: {imageOrientation: imageOrientation, pixelDenom: pixDenomSlider.value, imageFile: pixelatePreviewPage.imageFile}});
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
                    Qt.openUrlExternally(qsTr("http://magicphotos.sourceforge.net/help/android/help.html"));
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
