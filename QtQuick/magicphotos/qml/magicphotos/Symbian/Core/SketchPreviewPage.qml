import QtQuick 1.1
import com.nokia.symbian 1.0
import ImageEditor 1.0

Page {
    id:           sketchPreviewPage
    anchors.fill: parent

    property bool   openFileOnActivation: true

    property string openFileUrl:          ""

    onStatusChanged: {
        if (status === PageStatus.Active && openFileOnActivation && openFileUrl !== "") {
            openFileOnActivation = false;

            sketchPreviewGenerator.radius = gaussianRadiusSlider.value;

            sketchPreviewGenerator.openImage(openFileUrl);
        }
    }

    onOpenFileUrlChanged: {
        if (status === PageStatus.Active && openFileOnActivation && openFileUrl !== "") {
            openFileOnActivation = false;

            sketchPreviewGenerator.radius = gaussianRadiusSlider.value;

            sketchPreviewGenerator.openImage(openFileUrl);
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
                    source:           "../../images/busy_indicator.png"
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
            text:             "Apply"

            onClicked: {
                mainPageStack.push(Qt.resolvedUrl("SketchPage.qml"), {gaussianRadius: gaussianRadiusSlider.value, openFileUrl: openFileUrl});
            }
        }
    }

    ToolBar {
        id:             bottomToolBar
        anchors.bottom: parent.bottom
        z:              1

        tools: ToolBarLayout {
            ToolButton {
                iconSource: "../images/back.png"
                flat:       true

                onClicked: {
                    mainPageStack.pop();
                }
            }

            ToolButton {
                iconSource: "../images/help.png"
                flat:       true

                onClicked: {
                    mainPageStack.push(Qt.resolvedUrl("HelpPage.qml"));
                }
            }
        }
    }

    QueryDialog {
        id:               imageOpenFailedQueryDialog
        titleText:        "Error"
        icon:             "../../images/dialog_error.png"
        message:          "Could not open image"
        acceptButtonText: "OK"
    }
}
