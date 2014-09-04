import QtQuick 2.2
import QtQuick.Controls 1.2

Item {
    id: modeSelectionPage

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

                signal fileSelected(string image_file)
                signal fileSelectionCancelled()

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
                    var component;

                    if (mode === "DECOLORIZE"){
                        component = Qt.createComponent("DecolorizePage.qml");

                        if (component.status === Component.Ready) {
                            mainStackView.push({item: component.createObject(null), destroyOnPop: true, properties: {openImageFile: image_file}});
                        } else {
                            console.log(component.errorString());
                        }
                    } else if (mode === "SKETCH") {
                        component = Qt.createComponent("SketchPreviewPage.qml");

                        if (component.status === Component.Ready) {
                            mainStackView.push({item: component.createObject(null), destroyOnPop: true, properties: {openImageFile: image_file}});
                        } else {
                            console.log(component.errorString());
                        }
                    } else if (mode === "CARTOON") {
                        component = Qt.createComponent("CartoonPreviewPage.qml");

                        if (component.status === Component.Ready) {
                            mainStackView.push({item: component.createObject(null), destroyOnPop: true, properties: {openImageFile: image_file}});
                        } else {
                            console.log(component.errorString());
                        }
                    } else if (mode === "BLUR") {
                        component = Qt.createComponent("BlurPreviewPage.qml");

                        if (component.status === Component.Ready) {
                            mainStackView.push({item: component.createObject(null), destroyOnPop: true, properties: {openImageFile: image_file}});
                        } else {
                            console.log(component.errorString());
                        }
                    } else if (mode === "PIXELATE") {
                        component = Qt.createComponent("PixelatePreviewPage.qml");

                        if (component.status === Component.Ready) {
                            mainStackView.push({item: component.createObject(null), destroyOnPop: true, properties: {openImageFile: image_file}});
                        } else {
                            console.log(component.errorString());
                        }
                    } else if (mode === "RECOLOR") {
                        component = Qt.createComponent("RecolorPage.qml");

                        if (component.status === Component.Ready) {
                            mainStackView.push({item: component.createObject(null), destroyOnPop: true, properties: {openImageFile: image_file}});
                        } else {
                            console.log(component.errorString());
                        }
                    } else if (mode === "RETOUCH") {
                        component = Qt.createComponent("RetouchPage.qml");

                        if (component.status === Component.Ready) {
                            mainStackView.push({item: component.createObject(null), destroyOnPop: true, properties: {openImageFile: image_file}});
                        } else {
                            console.log(component.errorString());
                        }
                    }

                    AndroidGW.imageSelected.disconnect(modeSelectionItemDelegate.fileSelected);
                    AndroidGW.imageSelectionCancelled.disconnect(modeSelectionItemDelegate.fileSelectionCancelled);
                }

                onFileSelectionCancelled: {
                    AndroidGW.imageSelected.disconnect(modeSelectionItemDelegate.fileSelected);
                    AndroidGW.imageSelectionCancelled.disconnect(modeSelectionItemDelegate.fileSelectionCancelled);
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
                }

                Rectangle {
                    id:             modeButtonRectangle
                    anchors.bottom: parent.bottom
                    anchors.left:   parent.left
                    anchors.right:  parent.right
                    height:         modeButton.height + 16
                    color:          "transparent"

                    Button {
                        id:               modeButton
                        anchors.centerIn: parent
                        text:             qsTr("Open Image")

                        onClicked: {
                            AndroidGW.imageSelected.connect(modeSelectionItemDelegate.fileSelected);
                            AndroidGW.imageSelectionCancelled.connect(modeSelectionItemDelegate.fileSelectionCancelled);

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
        z:              1

        ToolButton {
            anchors.centerIn: parent
            iconSource:       "images/help.png"

            onClicked: {
                Qt.openUrlExternally(qsTr("http://m.youtube.com/"));
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
}
