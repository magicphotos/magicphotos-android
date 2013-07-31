import QtQuick 1.1
import com.nokia.meego 1.0
import ImageEditor 1.0

Page {
    id:           pixelatePreviewPage
    anchors.fill: parent

    property string openFileUrl: ""

    onStatusChanged: {
        if (status === PageStatus.Active && openFileUrl !== "") {
            pixelatePreviewGenerator.pixDenom = pixDenomSlider.value;

            pixelatePreviewGenerator.openImage(openFileUrl);
        }
    }

    onOpenFileUrlChanged: {
        if (status === PageStatus.Active && openFileUrl !== "") {
            pixelatePreviewGenerator.pixDenom = pixDenomSlider.value;

            pixelatePreviewGenerator.openImage(openFileUrl);
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
                    source:           "../../images/busy_indicator.png"
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
            text:             "Apply"

            onClicked: {
                mainPageStack.push(Qt.resolvedUrl("PixelatePage.qml"), {pixelDenom: pixDenomSlider.value, openFileUrl: openFileUrl});
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
