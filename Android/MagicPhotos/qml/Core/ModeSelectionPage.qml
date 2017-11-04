import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3

import "../Util.js" as UtilScript

Page {
    id: modeSelectionPage

    header: Pane {
        Material.background: Material.Red

        Label {
            anchors.centerIn: parent
            text:             qsTr("MagicPhotos")
            font.pointSize:   24
        }
    }

    footer: ToolBar {
        RowLayout {
            anchors.fill: parent

            ToolButton {
                implicitWidth:    UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                implicitHeight:   UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                visible:          !mainWindow.fullVersion
                Layout.alignment: Qt.AlignHCenter

                contentItem: Image {
                    source:   "qrc:/resources/images/tool_purchase.png"
                    fillMode: Image.PreserveAspectFit
                }

                onClicked: {
                    purchaseMessageDialog.open();
                }
            }

            ToolButton {
                implicitWidth:    UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                implicitHeight:   UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                Layout.alignment: Qt.AlignHCenter

                contentItem: Image {
                    source:   "qrc:/resources/images/tool_help.png"
                    fillMode: Image.PreserveAspectFit
                }

                onClicked: {
                    Qt.openUrlExternally(qsTr("http://magicphotos.sourceforge.net/help/android/help.html"));
                }
            }
        }
    }

    property string selectedMode: ""

    onSelectedModeChanged: {
        if (selectedMode !== "") {
            waitRectangle.visible = true;

            AndroidGW.showGallery();
        }
    }

    function fileSelected(image_file, image_orientation) {
        waitRectangle.visible = false;

        var component;

        if (selectedMode === "DECOLORIZE"){
            component = Qt.createComponent("DecolorizePage.qml");

            if (component.status === Component.Ready) {
                mainStackView.push(component, {imageOrientation: image_orientation, imageFile: image_file});
            } else {
                console.log(component.errorString());
            }
        } else if (selectedMode === "SKETCH") {
            component = Qt.createComponent("SketchPreviewPage.qml");

            if (component.status === Component.Ready) {
                mainStackView.push(component, {imageOrientation: image_orientation, imageFile: image_file});
            } else {
                console.log(component.errorString());
            }
        } else if (selectedMode === "CARTOON") {
            component = Qt.createComponent("CartoonPreviewPage.qml");

            if (component.status === Component.Ready) {
                mainStackView.push(component, {imageOrientation: image_orientation, imageFile: image_file});
            } else {
                console.log(component.errorString());
            }
        } else if (selectedMode === "BLUR") {
            component = Qt.createComponent("BlurPreviewPage.qml");

            if (component.status === Component.Ready) {
                mainStackView.push(component, {imageOrientation: image_orientation, imageFile: image_file});
            } else {
                console.log(component.errorString());
            }
        } else if (selectedMode === "PIXELATE") {
            component = Qt.createComponent("PixelatePreviewPage.qml");

            if (component.status === Component.Ready) {
                mainStackView.push(component, {imageOrientation: image_orientation, imageFile: image_file});
            } else {
                console.log(component.errorString());
            }
        } else if (selectedMode === "RECOLOR") {
            component = Qt.createComponent("RecolorPage.qml");

            if (component.status === Component.Ready) {
                mainStackView.push(component, {imageOrientation: image_orientation, imageFile: image_file});
            } else {
                console.log(component.errorString());
            }
        } else if (selectedMode === "RETOUCH") {
            component = Qt.createComponent("RetouchPage.qml");

            if (component.status === Component.Ready) {
                mainStackView.push(component, {imageOrientation: image_orientation, imageFile: image_file});
            } else {
                console.log(component.errorString());
            }
        }

        selectedMode = "";
    }

    function fileSelectionCancelled() {
        waitRectangle.visible = false;

        selectedMode = "";
    }

    function fileSelectionFailed() {
        waitRectangle.visible = false;

        selectedMode = "";

        imageSelectionFailedMessageDialog.open();
    }

    ScrollView {
        id:           scrollView
        anchors.fill: parent
        clip:         true

        GridLayout {
            width:         scrollView.width
            columns:       2
            rowSpacing:    UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 4)
            columnSpacing: UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 4)

            property int itemSize: (width - columnSpacing * (columns - 1)) / columns

            Image {
                source:                 "qrc:/resources/images/edit_mode_decolorize.png"
                fillMode:               Image.PreserveAspectFit
                Layout.preferredWidth:  parent.itemSize
                Layout.preferredHeight: parent.itemSize

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        modeSelectionPage.selectedMode = "DECOLORIZE";
                    }
                }
            }

            Image {
                source:                 "qrc:/resources/images/edit_mode_sketch.png"
                fillMode:               Image.PreserveAspectFit
                Layout.preferredWidth:  parent.itemSize
                Layout.preferredHeight: parent.itemSize

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        modeSelectionPage.selectedMode = "SKETCH";
                    }
                }
            }

            Image {
                source:                 "qrc:/resources/images/edit_mode_cartoon.png"
                fillMode:               Image.PreserveAspectFit
                Layout.preferredWidth:  parent.itemSize
                Layout.preferredHeight: parent.itemSize

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        modeSelectionPage.selectedMode = "CARTOON";
                    }
                }
            }

            Image {
                source:                 "qrc:/resources/images/edit_mode_blur.png"
                fillMode:               Image.PreserveAspectFit
                Layout.preferredWidth:  parent.itemSize
                Layout.preferredHeight: parent.itemSize

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        modeSelectionPage.selectedMode = "BLUR";
                    }
                }
            }

            Image {
                source:                 "qrc:/resources/images/edit_mode_pixelate.png"
                fillMode:               Image.PreserveAspectFit
                Layout.preferredWidth:  parent.itemSize
                Layout.preferredHeight: parent.itemSize

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        modeSelectionPage.selectedMode = "PIXELATE";
                    }
                }
            }

            Image {
                source:                 "qrc:/resources/images/edit_mode_recolor.png"
                fillMode:               Image.PreserveAspectFit
                Layout.preferredWidth:  parent.itemSize
                Layout.preferredHeight: parent.itemSize

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        modeSelectionPage.selectedMode = "RECOLOR";
                    }
                }
            }

            Image {
                source:                 "qrc:/resources/images/edit_mode_retouch.png"
                fillMode:               Image.PreserveAspectFit
                Layout.preferredWidth:  parent.itemSize
                Layout.preferredHeight: parent.itemSize

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        modeSelectionPage.selectedMode = "RETOUCH";
                    }
                }
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

    MessageDialog {
        id:              imageSelectionFailedMessageDialog
        title:           qsTr("Error")
        icon:            StandardIcon.Critical
        text:            qsTr("Could not open image")
        standardButtons: StandardButton.Ok
    }

    MessageDialog {
        id:              purchaseMessageDialog
        title:           qsTr("Info")
        icon:            StandardIcon.Information
        text:            qsTr("Tired of ads? No more ads with full version of this app. Do you want to purchase full version now?")
        standardButtons: StandardButton.Yes | StandardButton.No

        onYes: {
            mainWindow.purchaseFullVersion();
        }
    }

    Component.onCompleted: {
        AndroidGW.imageSelected.connect(fileSelected);
        AndroidGW.imageSelectionCancelled.connect(fileSelectionCancelled);
        AndroidGW.imageSelectionFailed.connect(fileSelectionFailed);
    }
}
