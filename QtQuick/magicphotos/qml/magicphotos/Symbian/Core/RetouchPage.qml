import QtQuick 1.1
import com.nokia.symbian 1.1
import ImageEditor 1.0

import "Util"

Page {
    id:           retouchPage
    anchors.fill: parent

    property string openFileUrl: ""
    property string saveFileUrl: ""

    Component.onCompleted: {
        retouchEditor.helperImageReady.connect(helper.helperImageReady);
    }

    onOpenFileUrlChanged: {
        if (openFileUrl !== "") {
            retouchEditor.openImage(openFileUrl);
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
            width:            64 * 4
            exclusive:        true
            checkedButton:    scrollModeButton

            Button {
                id:         scrollModeButton
                iconSource: "../images/mode_scroll.png"
                enabled:    false

                onCheckedChanged: {
                    if (checked) {
                        retouchEditor.mode          = RetouchEditor.ModeScroll;
                        editorFlickable.interactive = true;
                        editorPinchArea.enabled     = true;
                    }
                }
            }

            Button {
                id:         samplingPointModeButton
                iconSource: "../images/mode_sampling_point.png"
                enabled:    false

                onCheckedChanged: {
                    if (checked) {
                        retouchEditor.mode          = RetouchEditor.ModeSamplingPoint;
                        editorFlickable.interactive = false;
                        editorPinchArea.enabled     = false;
                    }
                }
            }

            Button {
                id:         cloneModeButton
                iconSource: "../images/mode_clone.png"
                enabled:    false

                onCheckedChanged: {
                    if (checked) {
                        retouchEditor.mode          = RetouchEditor.ModeClone;
                        editorFlickable.interactive = false;
                        editorPinchArea.enabled     = false;
                    }
                }
            }

            Button {
                id:         blurModeButton
                iconSource: "../images/mode_blur.png"
                enabled:    false

                onCheckedChanged: {
                    if (checked) {
                        retouchEditor.mode          = RetouchEditor.ModeBlur;
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
                    retouchEditor.width = contentWidth;

                    samplingPointImage.updatePosition();
                }
            }

            onContentHeightChanged: {
                if (contentHeight >= 0.0) {
                    retouchEditor.height = contentHeight;

                    samplingPointImage.updatePosition();
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

                Rectangle {
                    width:  retouchEditor.width
                    height: retouchEditor.height
                    clip:   true
                    color:  "transparent"

                    RetouchEditor {
                        id:         retouchEditor
                        helperSize: helper.width

                        onImageOpened: {
                            waitRectangle.visible = false;

                            saveToolButton.enabled = true;

                            scrollModeButton.enabled        = true;
                            samplingPointModeButton.enabled = true;
                            cloneModeButton.enabled         = true;
                            blurModeButton.enabled          = true;

                            editorFlickable.contentWidth         = width;
                            editorFlickable.contentHeight        = height;
                            editorFlickable.initialContentWidth  = width;
                            editorFlickable.initialContentHeight = height;
                        }

                        onImageOpenFailed: {
                            waitRectangle.visible = false;

                            saveToolButton.enabled = false;

                            scrollModeButton.enabled        = false;
                            samplingPointModeButton.enabled = false;
                            cloneModeButton.enabled         = false;
                            blurModeButton.enabled          = false;

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

                            if (event_type === RetouchEditor.MousePressed) {
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
                            } else if (event_type === RetouchEditor.MouseMoved) {
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
                            } else if (event_type === RetouchEditor.MouseReleased) {
                                helperRectangle.visible = false;
                            }
                        }
                    }

                    Image {
                        id:      samplingPointImage
                        width:   48
                        height:  48
                        source:  "../../images/sampling_point.png"
                        visible: retouchEditor.samplingPointValid

                        property int samplingPointX: retouchEditor.samplingPoint.x
                        property int samplingPointY: retouchEditor.samplingPoint.y

                        onSamplingPointXChanged: {
                            if (editorFlickable.initialContentWidth > 0.0) {
                                var scale = editorFlickable.contentWidth / editorFlickable.initialContentWidth;

                                x = samplingPointX * scale - width / 2;
                            }
                        }

                        onSamplingPointYChanged: {
                            if (editorFlickable.initialContentWidth > 0.0) {
                                var scale = editorFlickable.contentWidth / editorFlickable.initialContentWidth;

                                y = samplingPointY * scale - height / 2;
                            }
                        }

                        function updatePosition() {
                            if (editorFlickable.initialContentWidth > 0.0) {
                                var scale = editorFlickable.contentWidth / editorFlickable.initialContentWidth;

                                x = samplingPointX * scale - width  / 2;
                                y = samplingPointY * scale - height / 2;
                            }
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
                    if (retouchEditor.changed) {
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
                    retouchEditor.undo();
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
            retouchPage.saveFileUrl = file_url_path + "/" + file_name;

            retouchEditor.saveImage(retouchPage.saveFileUrl);
        }
    }
}
