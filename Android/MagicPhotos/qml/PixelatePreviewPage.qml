import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.1
import ImageEditor 1.0

Item {
    id:    pixelatePreviewPage
    focus: true

    property string openImageFile: ""

    Keys.onReleased: {
        if (event.key === Qt.Key_Back) {
            mainStackView.pop();

            event.accepted = true;
        }
    }

    onOpenImageFileChanged: {
        if (openImageFile !== "") {
            pixelatePreviewGenerator.pixDenom = pixDenomSlider.value;

            pixelatePreviewGenerator.openImage(openImageFile);
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

                imageOpenFailedQueryDialog.open();
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
        id:             pixDenomSliderRectangle
        anchors.bottom: applyButtonRectangle.top
        anchors.left:   parent.left
        anchors.right:  parent.right
        height:         pixDenomSlider.height + 16
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
        height:         applyButton.height + 16
        color:          "transparent"

        Button {
            id:               applyButton
            anchors.centerIn: parent
            enabled:          false
            text:             qsTr("Apply")

            onClicked: {
                var component = Qt.createComponent("PixelatePage.qml");

                if (component.status === Component.Ready) {
                    mainStackView.push({item: component.createObject(null), destroyOnPop: true, properties: {pixelDenom: pixDenomSlider.value, openImageFile: openImageFile}});
                } else {
                    console.log(component.errorString());
                }
            }
        }
    }

    ToolBar {
        id:             bottomToolBar
        anchors.bottom: parent.bottom
        z:              1

        RowLayout {
            anchors.fill: parent

            ToolButton {
                iconSource: "images/back.png"

                onClicked: {
                    mainStackView.pop();
                }
            }

            ToolButton {
                iconSource: "images/help.png"

                onClicked: {
                    Qt.openUrlExternally(qsTr("http://m.youtube.com/"));
                }
            }
        }
    }

    MessageDialog {
        id:              imageOpenFailedQueryDialog
        title:           qsTr("Error")
        icon:            StandardIcon.Critical
        text:            qsTr("Could not open image")
        standardButtons: StandardButton.Ok
    }
}
