import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.3
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2
import ImageEditor 1.0

import "../Util.js" as UtilScript

Page {
    id: pixelatePreviewPage

    header: Pane {
        Material.background: Material.Green

        Label {
            anchors.centerIn: parent
            text:             qsTr("Pixelate")
            font.pixelSize:   UtilScript.dp(UIHelper.screenDpi, 24)
            font.family:      "Helvetica"
        }
    }

    footer: ToolBar {
        RowLayout {
            anchors.fill: parent

            ToolButton {
                id:               applyToolButton
                implicitWidth:    UtilScript.dp(UIHelper.screenDpi, 48)
                implicitHeight:   UtilScript.dp(UIHelper.screenDpi, 48)
                enabled:          false
                Layout.alignment: Qt.AlignHCenter

                contentItem: Image {
                    source:   "qrc:/resources/images/tool_apply.png"
                    fillMode: Image.PreserveAspectFit
                }

                onClicked: {
                    var component = Qt.createComponent("PixelatePage.qml");

                    if (component.status === Component.Ready) {
                        mainStackView.push(component, {"imageOrientation": imageOrientation, "pixelDenom": pixDenomSlider.value, "imagePath": pixelatePreviewPage.imagePath});
                    } else {
                        console.error(component.errorString());
                    }
                }
            }
        }
    }

    property int imageOrientation: -1

    property string imagePath:     ""

    onImageOrientationChanged: {
        if (imageOrientation !== -1 && imagePath !== "") {
            pixelatePreviewGenerator.pixDenom = pixDenomSlider.value;

            pixelatePreviewGenerator.openImage(imagePath, imageOrientation);
        }
    }

    onImagePathChanged: {
        if (imageOrientation !== -1 && imagePath !== "") {
            pixelatePreviewGenerator.pixDenom = pixDenomSlider.value;

            pixelatePreviewGenerator.openImage(imagePath, imageOrientation);
        }
    }

    Keys.onReleased: {
        if (event.key === Qt.Key_Back) {
            mainStackView.pop();

            event.accepted = true;
        }
    }

    ColumnLayout {
        anchors.fill:    parent
        anchors.margins: UtilScript.dp(UIHelper.screenDpi, 16)
        spacing:         UtilScript.dp(UIHelper.screenDpi, 16)

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
                z:            1
                color:        "black"
                opacity:      0.75
                visible:      false

                BusyIndicator {
                    anchors.centerIn: parent
                    running:          parent.visible
                }

                MultiPointTouchArea {
                    anchors.fill: parent
                }
            }
        }

        Slider {
            id:               pixDenomSlider
            from:             32
            to:               192
            value:            112
            stepSize:         8.0
            enabled:          false
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
        text:            qsTr("Could not open image")
        standardButtons: StandardButton.Ok
    }
}
