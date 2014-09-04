import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.1
import ImageEditor 1.0

Item {
    id:    sketchPreviewPage
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
            sketchPreviewGenerator.radius = gaussianRadiusSlider.value;

            sketchPreviewGenerator.openImage(openImageFile);
        }
    }

    Rectangle {
        anchors.top:    parent.top
        anchors.bottom: gaussianRadiusSliderRectangle.top
        anchors.left:   parent.left
        anchors.right:  parent.right
        color:          "transparent"

        SketchPreviewGenerator {
            id:           sketchPreviewGenerator
            anchors.fill: parent

            property int waitRectangleUsageCounter: 0

            onImageOpened: {
                gaussianRadiusSlider.enabled = true;
                applyButton.enabled          = true;
            }

            onImageOpenFailed: {
                gaussianRadiusSlider.enabled = false;
                applyButton.enabled          = false;

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
        id:             gaussianRadiusSliderRectangle
        anchors.bottom: applyButtonRectangle.top
        anchors.left:   parent.left
        anchors.right:  parent.right
        height:         gaussianRadiusSlider.height + 16
        color:          "transparent"

        Slider {
            id:                     gaussianRadiusSlider
            anchors.verticalCenter: parent.verticalCenter
            anchors.left:           parent.left
            anchors.right:          parent.right
            enabled:                false
            minimumValue:           4
            maximumValue:           18
            value:                  11
            stepSize:               1.0

            onPressedChanged: {
                if (!pressed) {
                    sketchPreviewGenerator.radius = value;
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
                var component = Qt.createComponent("SketchPage.qml");

                if (component.status === Component.Ready) {
                    mainStackView.push({item: component.createObject(null), destroyOnPop: true, properties: {gaussianRadius: gaussianRadiusSlider.value, openImageFile: openImageFile}});
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
