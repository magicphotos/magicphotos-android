import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.3
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2

import "../../Util.js" as UtilScript

Page {
    id: previewPage

    header: Pane {
        topPadding:          previewPage.bannerViewHeight
        Material.background: Material.Green

        Label {
            anchors.centerIn: parent
            text:             previewPage.headerLabelText
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
                    previewPage.applyClicked();
                }
            }
        }
    }

    readonly property int bannerViewHeight:    AdMobHelper.bannerViewHeight

    readonly property var previewGenerator:    previewGeneratorLoader.item
    readonly property var controlColumnLayout: controlColumnLayoutLoader.item

    property string headerLabelText:           ""

    property var previewGeneratorComponent:    undefined
    property var controlColumnLayoutComponent: undefined

    signal applyClicked()

    Keys.onReleased: {
        if (event.key === Qt.Key_Back) {
            mainStackView.pop();

            event.accepted = true;
        }
    }

    function getControlValues() {
        var result = [];

        for (var i = 0; i < controlColumnLayout.children.length; i++) {
            if (controlColumnLayout.children[i] instanceof Slider) {
                result.push(controlColumnLayout.children[i].value);
            }
        }

        return result;
    }

    ColumnLayout {
        anchors.fill:    parent
        anchors.margins: UtilScript.dp(UIHelper.screenDpi, 16)
        spacing:         UtilScript.dp(UIHelper.screenDpi, 16)

        Rectangle {
            color:             "transparent"
            Layout.fillWidth:  true
            Layout.fillHeight: true

            Loader {
                id:              previewGeneratorLoader
                anchors.fill:    parent
                sourceComponent: previewPage.previewGeneratorComponent
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

        Loader {
            id:               controlColumnLayoutLoader
            sourceComponent:  previewPage.controlColumnLayoutComponent
            Layout.fillWidth: true

            onLoaded: {
                item.enabled = false;
            }
        }
    }

    MessageDialog {
        id:              imageOpenFailedMessageDialog
        title:           qsTr("Error")
        text:            qsTr("Could not open image")
        standardButtons: StandardButton.Ok
    }

    Connections {
        target: previewPage.previewGenerator

        onImageOpened: {
            applyToolButton.enabled                 = true;
            previewPage.controlColumnLayout.enabled = true;
        }

        onImageOpenFailed: {
            applyToolButton.enabled                 = false;
            previewPage.controlColumnLayout.enabled = false;

            imageOpenFailedMessageDialog.open();
        }

        onGenerationStarted: {
            if (typeof previewPage.previewGenerator.waitRectangleUsageCounter === "number") {
                previewPage.previewGenerator.waitRectangleUsageCounter = previewPage.previewGenerator.waitRectangleUsageCounter + 1;

                if (previewPage.previewGenerator.waitRectangleUsageCounter === 1) {
                    waitRectangle.visible = true;
                }
            }
        }

        onGenerationFinished: {
            if (typeof previewPage.previewGenerator.waitRectangleUsageCounter === "number") {
                if (previewPage.previewGenerator.waitRectangleUsageCounter === 1) {
                    waitRectangle.visible = false;
                }

                if (previewPage.previewGenerator.waitRectangleUsageCounter > 0) {
                    previewPage.previewGenerator.waitRectangleUsageCounter = previewPage.previewGenerator.waitRectangleUsageCounter - 1;
                }
            }
        }
    }
}
