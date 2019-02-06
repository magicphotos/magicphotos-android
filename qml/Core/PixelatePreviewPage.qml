import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.3
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.12
import ImageEditor 1.0

import "../Util.js" as UtilScript

Page {
    id: pixelatePreviewPage

    header: Pane {
        Material.background: Material.Green

        Label {
            anchors.centerIn: parent
            text:             qsTr("Pixelate")
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
                    var component = Qt.createComponent("PixelatePage.qml");

                    if (component.status === Component.Ready) {
                        mainStackView.push(component, {imageOrientation: imageOrientation, pixelDenom: pixDenomSlider.value, imageFile: pixelatePreviewPage.imageFile});
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

    ColumnLayout {
        anchors.fill:    parent
        anchors.margins: UtilScript.pt(16)
        spacing:         UtilScript.pt(16)

        Rectangle {
            color:             "transparent"
            Layout.fillWidth:  true
            Layout.fillHeight: true

            PixelatePreviewGenerator {
                id:           pixelatePreviewGenerator
                anchors.fill: parent

                property int waitRectangleUsageCounter: 0

                onImageOpened: {
                    pixDenomSlider.enabled  = true;
                    applyToolButton.enabled = true;
                }

                onImageOpenFailed: {
                    pixDenomSlider.enabled  = false;
                    applyToolButton.enabled = false;

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
            id:               pixDenomSlider
            enabled:          false
            from:             32
            to:               192
            value:            112
            stepSize:         8.0
            Layout.fillWidth: true

            onPressedChanged: {
                if (!pressed) {
                    pixelatePreviewGenerator.pixDenom = value;
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
