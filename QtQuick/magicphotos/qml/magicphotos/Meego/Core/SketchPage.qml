import QtQuick 1.1
import com.nokia.meego 1.0
import ImageEditor 1.0

import "Util"

Page {
    id:           sketchPage
    anchors.fill: parent

    property int    gaussianRadius: -1

    property string openFileUrl:    ""
    property string saveFileUrl:    ""

    Component.onCompleted: {
        sketchEditor.helperImageReady.connect(helper.helperImageReady);
    }

    onGaussianRadiusChanged: {
        if (gaussianRadius !== -1 && openFileUrl !== "") {
            sketchEditor.radius = gaussianRadius;

            sketchEditor.openImage(openFileUrl);
        }
    }

    onOpenFileUrlChanged: {
        if (gaussianRadius !== -1 && openFileUrl !== "") {
            sketchEditor.radius = gaussianRadius;

            sketchEditor.openImage(openFileUrl);
        }
    }

    Rectangle {
        id:            topButtonGroupRectangle
        anchors.top:   parent.top
        anchors.left:  parent.left
        anchors.right: parent.right
        height:        modeButtonRow.height
        z:             1
        color:         "transparent"

        ButtonRow {
            id:               modeButtonRow
            anchors.centerIn: parent
            width:            64 * 3
            exclusive:        true
            checkedButton:    scrollModeButton

            Button {
                id:         scrollModeButton
                iconSource: "../images/mode_scroll.png"
                enabled:    false

                onCheckedChanged: {
                    if (checked) {
                        sketchEditor.mode           = SketchEditor.ModeScroll;
                        editorFlickable.interactive = true;
                        editorPinchArea.enabled     = true;
                    }
                }
            }

            Button {
                id:         originalModeButton
                iconSource: "../images/mode_original.png"
                enabled:    false

                onCheckedChanged: {
                    if (checked) {
                        sketchEditor.mode           = SketchEditor.ModeOriginal;
                        editorFlickable.interactive = false;
                        editorPinchArea.enabled     = false;
                    }
                }
            }

            Button {
                id:         effectedModeButton
                iconSource: "../images/mode_effected.png"
                enabled:    false

                onCheckedChanged: {
                    if (checked) {
                        sketchEditor.mode           = SketchEditor.ModeEffected;
                        editorFlickable.interactive = false;
                        editorPinchArea.enabled     = false;
                    }
                }
            }
        }
    }

    Rectangle {
        id:             editorRectangle
        anchors.top:    topButtonGroupRectangle.bottom
        anchors.bottom: bottomToolBar.top
        anchors.left:   parent.left
        anchors.right:  parent.right
        color:          "transparent"

        Flickable {
            id:             editorFlickable
            anchors.fill:   parent
            boundsBehavior: Flickable.StopAtBounds

            property real initialContentWidth:  0.0
            property real initialContentHeight: 0.0

            onContentWidthChanged: {
                if (contentWidth >= 0.0) {
                    sketchEditor.width = contentWidth;
                }
            }

            onContentHeightChanged: {
                if (contentHeight >= 0.0) {
                    sketchEditor.height = contentHeight;
                }
            }

            PinchArea {
                id:             editorPinchArea
                anchors.fill:   parent
                pinch.dragAxis: Pinch.NoDrag

                onPinchUpdated: {
                    if (editorFlickable.initialContentWidth > 0.0) {
                        editorFlickable.contentX += pinch.previousCenter.x - pinch.center.x;
                        editorFlickable.contentY += pinch.previousCenter.y - pinch.center.y;

                        var scale = 1.0 + pinch.scale - pinch.previousScale;

                        if (editorFlickable.contentWidth * scale / editorFlickable.initialContentWidth >= 0.5 &&
                            editorFlickable.contentWidth * scale / editorFlickable.initialContentWidth <= 4.0) {
                            editorFlickable.resizeContent(editorFlickable.contentWidth * scale, editorFlickable.contentHeight * scale, pinch.center);
                        }
                    }
                }

                onPinchFinished: {
                    editorFlickable.returnToBounds();
                }

                SketchEditor {
                    id:         sketchEditor
                    helperSize: helper.width

                    onImageOpened: {
                        waitRectangle.visible = false;

                        saveToolButton.enabled = true;

                        scrollModeButton.enabled   = true;
                        originalModeButton.enabled = true;
                        effectedModeButton.enabled = true;

                        editorFlickable.contentWidth         = width;
                        editorFlickable.contentHeight        = height;
                        editorFlickable.initialContentWidth  = width;
                        editorFlickable.initialContentHeight = height;
                    }

                    onImageOpenFailed: {
                        waitRectangle.visible = false;

                        saveToolButton.enabled = false;

                        scrollModeButton.enabled   = false;
                        originalModeButton.enabled = false;
                        effectedModeButton.enabled = false;

                        imageOpenFailedQueryDialog.open();
                    }

                    onImageSaveFailed: {
                        imageSaveFailedQueryDialog.open();
                    }

                    onUndoAvailabilityChanged: {
                        if (available) {
                            undoToolButton.enabled = true;
                        } else {
                            undoToolButton.enabled = false;
                        }
                    }

                    onMouseEvent: {
                        var rect = mapToItem(editorRectangle, x, y);

                        if (event_type === SketchEditor.MousePressed) {
                            helperRectangle.visible = true;

                            if (rect.y < editorRectangle.height / 2) {
                                if (rect.x < editorRectangle.width / 2) {
                                    helperRectangle.anchors.left  = undefined;
                                    helperRectangle.anchors.right = editorRectangle.right;
                                } else {
                                    helperRectangle.anchors.right = undefined;
                                    helperRectangle.anchors.left  = editorRectangle.left;
                                }
                            }
                        } else if (event_type === SketchEditor.MouseMoved) {
                            helperRectangle.visible = true;

                            if (rect.y < editorRectangle.height / 2) {
                                if (rect.x < editorRectangle.width / 2) {
                                    helperRectangle.anchors.left  = undefined;
                                    helperRectangle.anchors.right = editorRectangle.right;
                                } else {
                                    helperRectangle.anchors.right = undefined;
                                    helperRectangle.anchors.left  = editorRectangle.left;
                                }
                            }
                        } else if (event_type === SketchEditor.MouseReleased) {
                            helperRectangle.visible = false;
                        }
                    }
                }
            }
        }

        Rectangle {
            id:           helperRectangle
            anchors.top:  parent.top
            anchors.left: parent.left
            width:        128
            height:       128
            z:            5
            visible:      false
            color:        "black"
            border.color: "white"
            border.width: 2

            Helper {
                id:           helper
                anchors.fill: parent
            }
        }

        Rectangle {
            id:           waitRectangle
            anchors.fill: parent
            z:            10
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

    ToolBar {
        id:             bottomToolBar
        anchors.bottom: parent.bottom
        z:              1

        tools: ToolBarLayout {
            ToolButton {
                iconSource: "../images/back.png"
                flat:       true

                onClicked: {
                    if (sketchEditor.changed) {
                        backQueryDialog.open();
                    } else {
                        mainPageStack.pop();
                    }
                }
            }

            ToolButton {
                id:         saveToolButton
                iconSource: "../images/save.png"
                flat:       true
                enabled:    false

                onClicked: {
                    if (saveFileUrl !== "") {
                        saveDialog.show(saveFileUrl);
                    } else {
                        saveDialog.show(openFileUrl);
                    }
                }
            }

            ToolButton {
                id:         undoToolButton
                iconSource: "../images/undo.png"
                flat:       true
                enabled:    false

                onClicked: {
                    sketchEditor.undo();
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

    QueryDialog {
        id:               imageSaveFailedQueryDialog
        titleText:        "Error"
        icon:             "../../images/dialog_error.png"
        message:          "Could not save image"
        acceptButtonText: "OK"
    }

    QueryDialog {
        id:               backQueryDialog
        titleText:        "Warning"
        icon:             "../../images/dialog_warning.png"
        message:          "Are you sure? Current image is not saved and will be lost."
        acceptButtonText: "Yes"
        rejectButtonText: "No"

        onAccepted: {
            mainPageStack.pop();
        }
    }

    SaveDialog {
        id: saveDialog

        onDone: {
            sketchPage.saveFileUrl = file_url_path + "/" + file_name;

            sketchEditor.saveImage(sketchPage.saveFileUrl);
        }
    }
}
