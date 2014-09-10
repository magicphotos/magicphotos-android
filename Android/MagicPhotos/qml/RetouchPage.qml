import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.1
import QtQuick.Window 2.1
import ImageEditor 1.0

import "Util"

import "Util.js" as UtilScript

Item {
    id:    retouchPage
    focus: true

    property string openImageFile: ""
    property string saveImageFile: ""

    Component.onCompleted: {
        retouchEditor.helperImageReady.connect(helper.helperImageReady);
    }

    Keys.onReleased: {
        if (event.key === Qt.Key_Back) {
            if (retouchEditor.changed) {
                backMessageDialog.open();
            } else {
                mainStackView.pop();
            }

            event.accepted = true;
        }
    }

    onOpenImageFileChanged: {
        if (openImageFile !== "") {
            retouchEditor.openImage(openImageFile);
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
                width:          UtilScript.mapSizeToDevice(Screen.pixelDensity, 48)
                height:         UtilScript.mapSizeToDevice(Screen.pixelDensity, 48)
                exclusiveGroup: buttonExclusiveGroup
                checkable:      true
                checked:        true
                enabled:        false

                style: ButtonStyle {
                    background: Rectangle {
                        implicitWidth:  control.width
                        implicitHeight: control.height
                        color:          control.checked ? "gray" : "lightgray"
                        radius:         UtilScript.mapSizeToDevice(Screen.pixelDensity, 4)

                        Image {
                            anchors.fill:    parent
                            anchors.margins: UtilScript.mapSizeToDevice(Screen.pixelDensity, 4)
                            source:          "images/mode_scroll.png"
                            fillMode:        Image.PreserveAspectFit
                        }
                    }
                }

                onCheckedChanged: {
                    if (checked) {
                        retouchEditor.mode          = RetouchEditor.ModeScroll;
                        editorFlickable.interactive = true;
                        editorPinchArea.enabled     = true;
                    }
                }
            }

            Button {
                id:             samplingPointModeButton
                width:          UtilScript.mapSizeToDevice(Screen.pixelDensity, 48)
                height:         UtilScript.mapSizeToDevice(Screen.pixelDensity, 48)
                exclusiveGroup: buttonExclusiveGroup
                checkable:      true
                enabled:        false

                style: ButtonStyle {
                    background: Rectangle {
                        implicitWidth:  control.width
                        implicitHeight: control.height
                        color:          control.checked ? "gray" : "lightgray"
                        radius:         UtilScript.mapSizeToDevice(Screen.pixelDensity, 4)

                        Image {
                            anchors.fill:    parent
                            anchors.margins: UtilScript.mapSizeToDevice(Screen.pixelDensity, 4)
                            source:          "images/mode_sampling_point.png"
                            fillMode:        Image.PreserveAspectFit
                        }
                    }
                }

                onCheckedChanged: {
                    if (checked) {
                        retouchEditor.mode          = RetouchEditor.ModeSamplingPoint;
                        editorFlickable.interactive = false;
                        editorPinchArea.enabled     = false;
                    }
                }
            }

            Button {
                id:             cloneModeButton
                width:          UtilScript.mapSizeToDevice(Screen.pixelDensity, 48)
                height:         UtilScript.mapSizeToDevice(Screen.pixelDensity, 48)
                exclusiveGroup: buttonExclusiveGroup
                checkable:      true
                enabled:        false

                style: ButtonStyle {
                    background: Rectangle {
                        implicitWidth:  control.width
                        implicitHeight: control.height
                        color:          control.checked ? "gray" : "lightgray"
                        radius:         UtilScript.mapSizeToDevice(Screen.pixelDensity, 4)

                        Image {
                            anchors.fill:    parent
                            anchors.margins: UtilScript.mapSizeToDevice(Screen.pixelDensity, 4)
                            source:          "images/mode_clone.png"
                            fillMode:        Image.PreserveAspectFit
                        }
                    }
                }

                onCheckedChanged: {
                    if (checked) {
                        retouchEditor.mode          = RetouchEditor.ModeClone;
                        editorFlickable.interactive = false;
                        editorPinchArea.enabled     = false;
                    }
                }
            }

            Button {
                id:             blurModeButton
                width:          UtilScript.mapSizeToDevice(Screen.pixelDensity, 48)
                height:         UtilScript.mapSizeToDevice(Screen.pixelDensity, 48)
                exclusiveGroup: buttonExclusiveGroup
                checkable:      true
                enabled:        false

                style: ButtonStyle {
                    background: Rectangle {
                        implicitWidth:  control.width
                        implicitHeight: control.height
                        color:          control.checked ? "gray" : "lightgray"
                        radius:         UtilScript.mapSizeToDevice(Screen.pixelDensity, 4)

                        Image {
                            anchors.fill:    parent
                            anchors.margins: UtilScript.mapSizeToDevice(Screen.pixelDensity, 4)
                            source:          "images/mode_blur.png"
                            fillMode:        Image.PreserveAspectFit
                        }
                    }
                }

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
                    samplingPointImage.updatePosition();
                }
            }

            onContentHeightChanged: {
                if (contentHeight >= 0.0) {
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

                onPinchStarted: {
                    editorFlickable.interactive = false;
                }

                onPinchFinished: {
                    editorFlickable.interactive = true;

                    editorFlickable.returnToBounds();
                }

                Rectangle {
                    width:  retouchEditor.width  * retouchEditor.scale
                    height: retouchEditor.height * retouchEditor.scale
                    clip:   true
                    color:  "transparent"

                    RetouchEditor {
                        id:                 retouchEditor
                        scale:              editorFlickable.contentWidth        > 0.0 &&
                                            editorFlickable.initialContentWidth > 0.0 ?
                                            editorFlickable.contentWidth / editorFlickable.initialContentWidth : 1.0
                        transformOrigin:    Item.TopLeft
                        helperSize:         helper.width
                        screenPixelDensity: Screen.pixelDensity

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

                            imageOpenFailedMessageDialog.open();
                        }

                        onImageSaveFailed: {
                            imageSaveFailedMessageDialog.open();
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
                        width:   UtilScript.mapSizeToDevice(Screen.pixelDensity, 48)
                        height:  UtilScript.mapSizeToDevice(Screen.pixelDensity, 48)
                        source:  "images/sampling_point.png"
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
            width:        UtilScript.mapSizeToDevice(Screen.pixelDensity, 128)
            height:       UtilScript.mapSizeToDevice(Screen.pixelDensity, 128)
            z:            5
            visible:      false
            color:        "black"
            border.color: "white"
            border.width: UtilScript.mapSizeToDevice(Screen.pixelDensity, 2)

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

    ToolBar {
        id:             bottomToolBar
        anchors.bottom: parent.bottom
        height:         UtilScript.mapSizeToDevice(Screen.pixelDensity, 48)
        z:              1

        RowLayout {
            anchors.fill: parent

            ToolButton {
                width:  UtilScript.mapSizeToDevice(Screen.pixelDensity, 48)
                height: UtilScript.mapSizeToDevice(Screen.pixelDensity, 48)

                style: ButtonStyle {
                    background: Rectangle {
                        implicitWidth:  control.width
                        implicitHeight: control.height
                        color:          "transparent"

                        Image {
                            anchors.fill:    parent
                            anchors.margins: UtilScript.mapSizeToDevice(Screen.pixelDensity, 4)
                            source:          "images/back.png"
                            fillMode:        Image.PreserveAspectFit
                        }
                    }
                }

                onClicked: {
                    if (retouchEditor.changed) {
                        backMessageDialog.open();
                    } else {
                        mainStackView.pop();
                    }
                }
            }

            ToolButton {
                id:      saveToolButton
                width:   UtilScript.mapSizeToDevice(Screen.pixelDensity, 48)
                height:  UtilScript.mapSizeToDevice(Screen.pixelDensity, 48)
                enabled: false

                style: ButtonStyle {
                    background: Rectangle {
                        implicitWidth:  control.width
                        implicitHeight: control.height
                        color:          "transparent"

                        Image {
                            anchors.fill:    parent
                            anchors.margins: UtilScript.mapSizeToDevice(Screen.pixelDensity, 4)
                            source:          "images/save.png"
                            fillMode:        Image.PreserveAspectFit
                        }
                    }
                }

                onClicked: {
                    if (saveImageFile !== "") {
                        saveDialog.show(saveImageFile);
                    } else {
                        saveDialog.show(openImageFile);
                    }
                }
            }

            ToolButton {
                id:      undoToolButton
                width:   UtilScript.mapSizeToDevice(Screen.pixelDensity, 48)
                height:  UtilScript.mapSizeToDevice(Screen.pixelDensity, 48)
                enabled: false

                style: ButtonStyle {
                    background: Rectangle {
                        implicitWidth:  control.width
                        implicitHeight: control.height
                        color:          "transparent"

                        Image {
                            anchors.fill:    parent
                            anchors.margins: UtilScript.mapSizeToDevice(Screen.pixelDensity, 4)
                            source:          "images/undo.png"
                            fillMode:        Image.PreserveAspectFit
                        }
                    }
                }

                onClicked: {
                    retouchEditor.undo();
                }
            }

            ToolButton {
                width:  UtilScript.mapSizeToDevice(Screen.pixelDensity, 48)
                height: UtilScript.mapSizeToDevice(Screen.pixelDensity, 48)

                style: ButtonStyle {
                    background: Rectangle {
                        implicitWidth:  control.width
                        implicitHeight: control.height
                        color:          "transparent"

                        Image {
                            anchors.fill:    parent
                            anchors.margins: UtilScript.mapSizeToDevice(Screen.pixelDensity, 4)
                            source:          "images/help.png"
                            fillMode:        Image.PreserveAspectFit
                        }
                    }
                }

                onClicked: {
                    Qt.openUrlExternally(qsTr("http://m.youtube.com/"));
                }
            }
        }
    }

    MessageDialog {
        id:              imageOpenFailedMessageDialog
        title:           qsTr("Error")
        icon:            StandardIcon.Critical
        text:            qsTr("Could not open image")
        standardButtons: StandardButton.Ok
    }

    MessageDialog {
        id:              imageSaveFailedMessageDialog
        title:           qsTr("Error")
        icon:            StandardIcon.Critical
        text:            qsTr("Could not save image")
        standardButtons: StandardButton.Ok
    }

    MessageDialog {
        id:              backMessageDialog
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
            retouchPage.focus         = true;
            retouchPage.saveImageFile = file_path + "/" + file_name;

            retouchEditor.saveImage(retouchPage.saveImageFile);

            AndroidGW.refreshGallery(retouchPage.saveImageFile);
        }

        onCancelPressed: {
            retouchPage.focus = true;
        }
    }
}
