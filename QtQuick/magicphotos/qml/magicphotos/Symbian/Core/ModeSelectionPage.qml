import QtQuick 1.1
import com.nokia.symbian 1.1

Page {
    id:           modeSelectionPage
    anchors.fill: parent

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
                    mode:     "DECOLORIZE"
                    modeName: "DECOLORIZE"
                    image:    "../../images/edit_mode_decolorize.png"
                }

                ListElement {
                    mode:     "SKETCH"
                    modeName: "SKETCH"
                    image:    "../../images/edit_mode_sketch.png"
                }

                ListElement {
                    mode:     "CARTOON"
                    modeName: "CARTOON"
                    image:    "../../images/edit_mode_cartoon.png"
                }

                ListElement {
                    mode:     "BLUR"
                    modeName: "BLUR"
                    image:    "../../images/edit_mode_blur.png"
                }

                ListElement {
                    mode:     "PIXELATE"
                    modeName: "PIXELATE"
                    image:    "../../images/edit_mode_pixelate.png"
                }

                ListElement {
                    mode:     "RECOLOR"
                    modeName: "RECOLOR"
                    image:    "../../images/edit_mode_recolor.png"
                }

                ListElement {
                    mode:     "RETOUCH"
                    modeName: "RETOUCH"
                    image:    "../../images/edit_mode_retouch.png"
                }
            }

            delegate: Rectangle {
                id:     modeSelectionItemDelegate
                width:  modeSelectionListView.width
                height: modeSelectionListView.height
                color:  "transparent"

                signal fileSelected(string file_url)

                onFileSelected: {
                    mainPageStack.pop();

                    if (mode === "DECOLORIZE") {
                        mainPageStack.push(Qt.resolvedUrl("DecolorizePage.qml"), {openFileUrl: file_url});
                    } else if (mode === "SKETCH") {
                        mainPageStack.push(Qt.resolvedUrl("SketchPreviewPage.qml"), {openFileUrl: file_url});
                    } else if (mode === "CARTOON") {
                        mainPageStack.push(Qt.resolvedUrl("CartoonPreviewPage.qml"), {openFileUrl: file_url});
                    } else if (mode === "BLUR") {
                        mainPageStack.push(Qt.resolvedUrl("BlurPreviewPage.qml"), {openFileUrl: file_url});
                    } else if (mode === "PIXELATE") {
                        mainPageStack.push(Qt.resolvedUrl("PixelatePreviewPage.qml"), {openFileUrl: file_url});
                    } else if (mode === "RECOLOR") {
                        mainPageStack.push(Qt.resolvedUrl("RecolorPage.qml"), {openFileUrl: file_url});
                    } else if (mode === "RETOUCH") {
                        mainPageStack.push(Qt.resolvedUrl("RetouchPage.qml"), {openFileUrl: file_url});
                    }
                }

                Text {
                    id:                  modeNameText
                    anchors.top:         parent.top
                    anchors.left:        parent.left
                    anchors.right:       parent.right
                    horizontalAlignment: Text.AlignHLeft
                    color:               "white"
                    font.pointSize:      14
                    text:                modeName
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
                        text:             "Open Image"

                        onClicked: {
                            mainPageStack.push(Qt.resolvedUrl("FileOpenPage.qml"), {caller: modeSelectionItemDelegate});
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

        tools: ToolBarLayout {
            ToolButton {
                iconSource: "../../images/close.png"

                onClicked: {
                    Qt.quit();
                }
            }

            ToolButton {
                iconSource: "../../images/help.png"

                onClicked: {
                    mainPageStack.push(Qt.resolvedUrl("HelpPage.qml"));
                }
            }
        }
    }
}
