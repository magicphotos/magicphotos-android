import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.1

import "Util.js" as UtilScript

Item {
    id: modeSelectionPage

    function playModeChangeSuggestionAnimation() {
        if (AppSettings.showModeChangeSuggestion) {
            modeChangeSuggestionAnimationTimer.start();

            AppSettings.showModeChangeSuggestion = false;
        }
    }

    function showPromoPopup() {
        if (AndroidGW.getPromoFullVersion()) {
            if (AppSettings.showPromoPopup) {
                promoPopupRectangle.visible = true;

                AppSettings.showPromoPopup = false;
            }
        }
    }

    Keys.onReleased: {
        if (event.key === Qt.Key_Back) {
            if (promoPopupRectangle.visible) {
                promoPopupRectangle.visible = false;

                event.accepted = true;
            }
        }
    }

    ListView {
        id:                 modeSelectionListView
        anchors.top:        parent.top
        anchors.bottom:     bottomToolBar.top
        anchors.left:       parent.left
        anchors.right:      parent.right
        orientation:        ListView.Horizontal
        highlightRangeMode: ListView.StrictlyEnforceRange
        snapMode:           ListView.SnapOneItem
        cacheBuffer:        width * modeSelectionListModel.count
        model:              modeSelectionVisualDataModel

        VisualDataModel {
            id: modeSelectionVisualDataModel

            model: ListModel {
                id: modeSelectionListModel

                ListElement {
                    mode:  "DECOLORIZE"
                    image: "images/edit_mode_decolorize.png"
                }

                ListElement {
                    mode:  "SKETCH"
                    image: "images/edit_mode_sketch.png"
                }

                ListElement {
                    mode:  "CARTOON"
                    image: "images/edit_mode_cartoon.png"
                }

                ListElement {
                    mode:  "BLUR"
                    image: "images/edit_mode_blur.png"
                }

                ListElement {
                    mode:  "PIXELATE"
                    image: "images/edit_mode_pixelate.png"
                }

                ListElement {
                    mode:  "RECOLOR"
                    image: "images/edit_mode_recolor.png"
                }

                ListElement {
                    mode:  "RETOUCH"
                    image: "images/edit_mode_retouch.png"
                }
            }

            delegate: Rectangle {
                id:     modeSelectionItemDelegate
                width:  modeSelectionListView.width
                height: modeSelectionListView.height
                color:  "transparent"

                property string itemMode: mode

                signal fileSelected(string image_file, int image_orientation)
                signal fileSelectionCancelled()
                signal fileSelectionFailed()

                onItemModeChanged: {
                    if (itemMode === "DECOLORIZE") {
                        modeNameText.text = qsTr("DECOLORIZE");
                    } else if (itemMode === "SKETCH") {
                        modeNameText.text = qsTr("SKETCH");
                    } else if (itemMode === "CARTOON") {
                        modeNameText.text = qsTr("CARTOON");
                    } else if (itemMode === "BLUR") {
                        modeNameText.text = qsTr("BLUR");
                    } else if (itemMode === "PIXELATE") {
                        modeNameText.text = qsTr("PIXELATE");
                    } else if (itemMode === "RECOLOR") {
                        modeNameText.text = qsTr("RECOLOR");
                    } else if (itemMode === "RETOUCH") {
                        modeNameText.text = qsTr("RETOUCH");
                    }
                }

                onFileSelected: {
                    waitRectangle.visible = false;

                    AndroidGW.imageSelected.disconnect(modeSelectionItemDelegate.fileSelected);
                    AndroidGW.imageSelectionCancelled.disconnect(modeSelectionItemDelegate.fileSelectionCancelled);
                    AndroidGW.imageSelectionFailed.disconnect(modeSelectionItemDelegate.fileSelectionFailed);

                    var component;

                    if (mode === "DECOLORIZE"){
                        component = Qt.createComponent("DecolorizePage.qml");

                        if (component.status === Component.Ready) {
                            mainStackView.push({item: component.createObject(null), destroyOnPop: true, properties: {imageOrientation: image_orientation, imageFile: image_file}});
                        } else {
                            console.log(component.errorString());
                        }
                    } else if (mode === "SKETCH") {
                        component = Qt.createComponent("SketchPreviewPage.qml");

                        if (component.status === Component.Ready) {
                            mainStackView.push({item: component.createObject(null), destroyOnPop: true, properties: {imageOrientation: image_orientation, imageFile: image_file}});
                        } else {
                            console.log(component.errorString());
                        }
                    } else if (mode === "CARTOON") {
                        component = Qt.createComponent("CartoonPreviewPage.qml");

                        if (component.status === Component.Ready) {
                            mainStackView.push({item: component.createObject(null), destroyOnPop: true, properties: {imageOrientation: image_orientation, imageFile: image_file}});
                        } else {
                            console.log(component.errorString());
                        }
                    } else if (mode === "BLUR") {
                        component = Qt.createComponent("BlurPreviewPage.qml");

                        if (component.status === Component.Ready) {
                            mainStackView.push({item: component.createObject(null), destroyOnPop: true, properties: {imageOrientation: image_orientation, imageFile: image_file}});
                        } else {
                            console.log(component.errorString());
                        }
                    } else if (mode === "PIXELATE") {
                        component = Qt.createComponent("PixelatePreviewPage.qml");

                        if (component.status === Component.Ready) {
                            mainStackView.push({item: component.createObject(null), destroyOnPop: true, properties: {imageOrientation: image_orientation, imageFile: image_file}});
                        } else {
                            console.log(component.errorString());
                        }
                    } else if (mode === "RECOLOR") {
                        component = Qt.createComponent("RecolorPage.qml");

                        if (component.status === Component.Ready) {
                            mainStackView.push({item: component.createObject(null), destroyOnPop: true, properties: {imageOrientation: image_orientation, imageFile: image_file}});
                        } else {
                            console.log(component.errorString());
                        }
                    } else if (mode === "RETOUCH") {
                        component = Qt.createComponent("RetouchPage.qml");

                        if (component.status === Component.Ready) {
                            mainStackView.push({item: component.createObject(null), destroyOnPop: true, properties: {imageOrientation: image_orientation, imageFile: image_file}});
                        } else {
                            console.log(component.errorString());
                        }
                    }
                }

                onFileSelectionCancelled: {
                    waitRectangle.visible = false;

                    AndroidGW.imageSelected.disconnect(modeSelectionItemDelegate.fileSelected);
                    AndroidGW.imageSelectionCancelled.disconnect(modeSelectionItemDelegate.fileSelectionCancelled);
                    AndroidGW.imageSelectionFailed.disconnect(modeSelectionItemDelegate.fileSelectionFailed);
                }

                onFileSelectionFailed: {
                    waitRectangle.visible = false;

                    AndroidGW.imageSelected.disconnect(modeSelectionItemDelegate.fileSelected);
                    AndroidGW.imageSelectionCancelled.disconnect(modeSelectionItemDelegate.fileSelectionCancelled);
                    AndroidGW.imageSelectionFailed.disconnect(modeSelectionItemDelegate.fileSelectionFailed);

                    imageSelectionFailedMessageDialog.open();
                }

                Text {
                    id:                  modeNameText
                    anchors.top:         parent.top
                    anchors.left:        parent.left
                    anchors.right:       parent.right
                    horizontalAlignment: Text.AlignHLeft
                    color:               "white"
                    font.pointSize:      24
                }

                Image {
                    anchors.top:    modeNameText.bottom
                    anchors.bottom: modeButtonRectangle.top
                    anchors.left:   parent.left
                    anchors.right:  parent.right
                    fillMode:       Image.PreserveAspectFit
                    smooth:         true
                    source:         image

                    MouseArea {
                        anchors.fill: parent

                        onClicked: {
                            waitRectangle.visible = true;

                            AndroidGW.imageSelected.connect(modeSelectionItemDelegate.fileSelected);
                            AndroidGW.imageSelectionCancelled.connect(modeSelectionItemDelegate.fileSelectionCancelled);
                            AndroidGW.imageSelectionFailed.connect(modeSelectionItemDelegate.fileSelectionFailed);

                            AndroidGW.showGallery();
                        }
                    }
                }

                Rectangle {
                    id:             modeButtonRectangle
                    anchors.bottom: parent.bottom
                    anchors.left:   parent.left
                    anchors.right:  parent.right
                    height:         modeButton.height + UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 16)
                    color:          "transparent"

                    Button {
                        id:               modeButton
                        anchors.centerIn: parent
                        text:             qsTr("Open Image")

                        style: ButtonStyle {
                            background: Rectangle {
                                implicitWidth:  control.width
                                implicitHeight: control.height
                                color:          "lightgray"
                                radius:         UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 4)
                            }

                            label: Text {
                                color: "black"
                                text:  control.text
                            }
                        }

                        onClicked: {
                            waitRectangle.visible = true;

                            AndroidGW.imageSelected.connect(modeSelectionItemDelegate.fileSelected);
                            AndroidGW.imageSelectionCancelled.connect(modeSelectionItemDelegate.fileSelectionCancelled);
                            AndroidGW.imageSelectionFailed.connect(modeSelectionItemDelegate.fileSelectionFailed);

                            AndroidGW.showGallery();
                        }
                    }
                }
            }
        }
    }

    ToolBar {
        id:             bottomToolBar
        anchors.bottom: parent.bottom
        height:         UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
        z:              1

        style: ToolBarStyle {
            background: Rectangle {
                color: "lightgray"
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
                Qt.openUrlExternally(qsTr("http://magicphotos.sourceforge.net/help/android/help.html"));
            }
        }
    }

    Rectangle {
        id:           promoPopupRectangle
        anchors.fill: parent
        z:            5
        color:        "transparent"
        visible:      false

        MouseArea {
            anchors.fill: parent

            Rectangle {
                anchors.centerIn: parent
                width:            promoPopupLayout.width  + UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 16)
                height:           promoPopupLayout.height + UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 16)
                color:            "white"

                ColumnLayout {
                    id:               promoPopupLayout
                    anchors.centerIn: parent
                    spacing:          UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 16)

                    Image {
                        id:                       promoImage
                        anchors.horizontalCenter: parent.horizontalCenter
                        source:                   "images/promo_myappfree.png"
                        fillMode:                 Image.PreserveAspectFit
                        Layout.maximumWidth:      promoPopupRectangle.width  - UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 16) * 2
                        Layout.maximumHeight:     promoPopupRectangle.height - UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 16) * 4 -
                                                                               promoPopupText.height - promoPopupButton.height
                    }

                    Text {
                        id:                       promoPopupText
                        anchors.horizontalCenter: parent.horizontalCenter
                        color:                    "black"
                        text:                     qsTr("Content unlocked!")
                        horizontalAlignment:      Text.AlignHCenter
                        wrapMode:                 Text.Wrap
                        Layout.maximumWidth:      promoPopupRectangle.width - UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 16) * 2
                    }

                    Button {
                        id:                       promoPopupButton
                        anchors.horizontalCenter: parent.horizontalCenter
                        text:                     qsTr("Thank you!")

                        style: ButtonStyle {
                            background: Rectangle {
                                implicitWidth:  control.width
                                implicitHeight: control.height
                                color:          "#00BCF2"
                            }

                            label: Text {
                                color: "black"
                                text:  control.text
                            }
                        }

                        onClicked: {
                            promoPopupRectangle.visible = false;
                        }
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

    SequentialAnimation {
        id: modeChangeSuggestionPropertyAnimation

        PropertyAnimation {
            target:      modeSelectionListView
            property:    "contentX"
            from:        0
            to:          modeSelectionListView.width * (modeSelectionListView.count - 1)
            easing.type: Easing.InOutExpo
            duration:    2000
        }

        PropertyAnimation {
            target:      modeSelectionListView
            property:    "contentX"
            from:        modeSelectionListView.width * (modeSelectionListView.count - 1)
            to:          0
            easing.type: Easing.InOutExpo
            duration:    2000
        }
    }

    Timer {
        id:       modeChangeSuggestionAnimationTimer
        interval: 500

        onTriggered: {
            modeChangeSuggestionPropertyAnimation.start();
        }
    }

    MessageDialog {
        id:              imageSelectionFailedMessageDialog
        title:           qsTr("Error")
        icon:            StandardIcon.Critical
        text:            qsTr("Could not open image")
        standardButtons: StandardButton.Ok
    }
}
