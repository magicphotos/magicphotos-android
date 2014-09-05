import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.1
import ImageEditor 1.0

import "Util"

Item {
    id: recolorPage

    property string openImageFile: ""
    property string saveImageFile: ""

    Component.onCompleted: {
        recolorEditor.helperImageReady.connect(helper.helperImageReady);
    }

    Keys.onReleased: {
        if (event.key === Qt.Key_Back) {
            if (recolorEditor.changed) {
                backQueryDialog.open();
            } else {
                mainStackView.pop();
            }

            event.accepted = true;
        }
    }

    onOpenImageFileChanged: {
        if (openImageFile !== "") {
            recolorEditor.openImage(openImageFile);
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

        ExclusiveGroup {
            id: buttonExclusiveGroup
        }

        Row {
            id:               modeButtonRow
            anchors.centerIn: parent

            Button {
                id:             scrollModeButton
                iconSource:     "images/mode_scroll.png"
                width:          80
                exclusiveGroup: buttonExclusiveGroup
                checkable:      true
                checked:        true
                enabled:        false

                onCheckedChanged: {
                    if (checked) {
                        recolorEditor.mode          = RecolorEditor.ModeScroll;
                        editorFlickable.interactive = true;
                        editorPinchArea.enabled     = true;
                    }
                }
            }

            Button {
                id:             originalModeButton
                iconSource:     "images/mode_original.png"
                width:          80
                exclusiveGroup: buttonExclusiveGroup
                checkable:      true
                enabled:        false

                onCheckedChanged: {
                    if (checked) {
                        recolorEditor.mode          = RecolorEditor.ModeOriginal;
                        editorFlickable.interactive = false;
                        editorPinchArea.enabled     = false;
                    }
                }
            }

            Button {
                id:             effectedModeButton
                iconSource:     "images/mode_effected.png"
                width:          80
                exclusiveGroup: buttonExclusiveGroup
                checkable:      true
                enabled:        false

                onCheckedChanged: {
                    if (checked) {
                        recolorEditor.mode          = RecolorEditor.ModeEffected;
                        editorFlickable.interactive = false;
                        editorPinchArea.enabled     = false;
                    }
                }
            }

            Button {
                id:             hueSelectionModeButton
                iconSource:     "images/mode_hue_selection.png"
                width:          80
                exclusiveGroup: buttonExclusiveGroup
                checkable:      true
                enabled:        false

                onCheckedChanged: {
                    if (checked) {
                        recolorEditor.mode          = RecolorEditor.ModeEffected;
                        editorFlickable.interactive = false;
                        editorPinchArea.enabled     = false;
                        hueZoneRectangle.visible    = true;
                    } else {
                        hueZoneRectangle.visible    = false;
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

                onPinchStarted: {
                    editorFlickable.interactive = false;
                }

                onPinchFinished: {
                    editorFlickable.interactive = true;

                    editorFlickable.returnToBounds();
                }

                RecolorEditor {
                    id:              recolorEditor
                    scale:           editorFlickable.contentWidth        > 0.0 &&
                                     editorFlickable.initialContentWidth > 0.0 ?
                                     editorFlickable.contentWidth / editorFlickable.initialContentWidth : 1.0
                    transformOrigin: Item.TopLeft
                    helperSize:      helper.width
                    hue:             180

                    onImageOpened: {
                        waitRectangle.visible = false;

                        saveToolButton.enabled = true;

                        scrollModeButton.enabled       = true;
                        originalModeButton.enabled     = true;
                        effectedModeButton.enabled     = true;
                        hueSelectionModeButton.enabled = true;

                        editorFlickable.contentWidth         = width;
                        editorFlickable.contentHeight        = height;
                        editorFlickable.initialContentWidth  = width;
                        editorFlickable.initialContentHeight = height;
                    }

                    onImageOpenFailed: {
                        waitRectangle.visible = false;

                        saveToolButton.enabled = false;

                        scrollModeButton.enabled       = false;
                        originalModeButton.enabled     = false;
                        effectedModeButton.enabled     = false;
                        hueSelectionModeButton.enabled = false;

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

                        if (event_type === RecolorEditor.MousePressed) {
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
                        } else if (event_type === RecolorEditor.MouseMoved) {
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
                        } else if (event_type === RecolorEditor.MouseReleased) {
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
                    source:           "images/busy_indicator.png"
                }
            }
        }
    }

    Rectangle {
        id:                     hueZoneRectangle
        anchors.right:          parent.right
        anchors.verticalCenter: parent.verticalCenter
        width:                  48
        height:                 240
        z:                      1
        color:                  "transparent"
        border.color:           "black"
        border.width:           1
        visible:                false

        gradient: Gradient {
            GradientStop { position: 1.0;  color: "#FF0000" }
            GradientStop { position: 0.85; color: "#FFFF00" }
            GradientStop { position: 0.76; color: "#00FF00" }
            GradientStop { position: 0.5;  color: "#00FFFF" }
            GradientStop { position: 0.33; color: "#0000FF" }
            GradientStop { position: 0.16; color: "#FF00FF" }
            GradientStop { position: 0.0;  color: "#FF0000" }
        }

        MouseArea {
            anchors.fill: parent

            Rectangle {
                id:            hueSliderRectangle
                anchors.left:  parent.left
                anchors.right: parent.right
                height:        6
                y:             (parent.height - height) * 0.5
                z:             2
                color:         "transparent"
                border.color:  "black"
                border.width:  2
            }

            onPositionChanged: {
                hueSliderRectangle.y = Math.max(0, Math.min(height - hueSliderRectangle.height, mouse.y));

                recolorEditor.hue = (height - Math.max(0, Math.min(height, mouse.y))) * (359 / height);
            }

            onPressed: {
                hueSliderRectangle.y = Math.max(0, Math.min(height - hueSliderRectangle.height, mouse.y));

                recolorEditor.hue = (height - Math.max(0, Math.min(height, mouse.y))) * (359 / height);
            }
        }
    }

    ToolBar {
        id:             bottomToolBar
        anchors.bottom: parent.bottom
        z:              1

        RowLayout {
            anchors.fill: parent

            ToolButton {
                iconSource: "images/back.png"

                onClicked: {
                    if (recolorEditor.changed) {
                        backQueryDialog.open();
                    } else {
                        mainStackView.pop();
                    }
                }
            }

            ToolButton {
                id:         saveToolButton
                iconSource: "images/save.png"
                enabled:    false

                onClicked: {
                    if (saveImageFile !== "") {
                        saveDialog.show(saveImageFile);
                    } else {
                        saveDialog.show(openImageFile);
                    }
                }
            }

            ToolButton {
                id:         undoToolButton
                iconSource: "images/undo.png"
                enabled:    false

                onClicked: {
                    recolorEditor.undo();
                }
            }

            ToolButton {
                iconSource: "images/help.png"

                onClicked: {
                    Qt.openUrlExternally(qsTr("http://m.youtube.com/"));
                }
            }
        }
    }

    MessageDialog {
        id:              imageOpenFailedQueryDialog
        title:           qsTr("Error")
        icon:            StandardIcon.Critical
        text:            qsTr("Could not open image")
        standardButtons: StandardButton.Ok
    }

    MessageDialog {
        id:              imageSaveFailedQueryDialog
        title:           qsTr("Error")
        icon:            StandardIcon.Critical
        text:            qsTr("Could not save image")
        standardButtons: StandardButton.Ok
    }

    MessageDialog {
        id:              backQueryDialog
        title:           qsTr("Warning")
        icon:            StandardIcon.Warning
        text:            qsTr("Are you sure? Current image is not saved and will be lost.")
        standardButtons: StandardButton.Yes | StandardButton.No

        onYes: {
            mainStackView.pop();
        }
    }

    SaveDialog {
        id: saveDialog

        onOkPressed: {
            recolorPage.focus         = true;
            recolorPage.saveImageFile = file_path + "/" + file_name;

            recolorEditor.saveImage(recolorPage.saveImageFile);

            AndroidGW.refreshGallery(recolorPage.saveImageFile);
        }

        onCancelPressed: {
            recolorPage.focus = true;
        }
    }
}
