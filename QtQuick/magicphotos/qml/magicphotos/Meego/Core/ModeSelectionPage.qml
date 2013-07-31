import QtQuick 1.1
import com.nokia.meego 1.0

import "../Settings.js" as SettingsScript

Page {
    id:           modeSelectionPage
    anchors.fill: parent

    onStatusChanged: {
        if (status === PageStatus.Active) {
            if (SettingsScript.getSetting("ShowModeChangeSuggestion", "TRUE") === "TRUE") {
                modeChangeSuggestionPropertyAnimation.start();

                SettingsScript.setSetting("ShowModeChangeSuggestion", "FALSE");
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
                    if (mode === "DECOLORIZE") {
                        mainPageStack.replace(Qt.resolvedUrl("DecolorizePage.qml"), {openFileUrl: file_url});
                    } else if (mode === "SKETCH") {
                        mainPageStack.replace(Qt.resolvedUrl("SketchPreviewPage.qml"), {openFileUrl: file_url});
                    } else if (mode === "CARTOON") {
                        mainPageStack.replace(Qt.resolvedUrl("CartoonPreviewPage.qml"), {openFileUrl: file_url});
                    } else if (mode === "BLUR") {
                        mainPageStack.replace(Qt.resolvedUrl("BlurPreviewPage.qml"), {openFileUrl: file_url});
                    } else if (mode === "PIXELATE") {
                        mainPageStack.replace(Qt.resolvedUrl("PixelatePreviewPage.qml"), {openFileUrl: file_url});
                    } else if (mode === "RECOLOR") {
                        mainPageStack.replace(Qt.resolvedUrl("RecolorPage.qml"), {openFileUrl: file_url});
                    } else if (mode === "RETOUCH") {
                        mainPageStack.replace(Qt.resolvedUrl("RetouchPage.qml"), {openFileUrl: file_url});
                    } else {
                        mainPageStack.pop();
                    }
                }

                Text {
                    id:                  modeNameText
                    anchors.top:         parent.top
                    anchors.left:        parent.left
                    anchors.right:       parent.right
                    horizontalAlignment: Text.AlignHLeft
                    color:               "white"
                    font.pointSize:      24
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
                iconSource: "../images/close.png"
                flat:       true

                onClicked: {
                    Qt.quit();
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
