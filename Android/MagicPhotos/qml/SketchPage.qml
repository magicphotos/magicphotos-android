import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.1
import ImageEditor 1.0

import "Util.js" as UtilScript

Item {
    id:    sketchPage
    focus: true

    property bool   shareActionActive: false

    property int    imageOrientation:  -1
    property int    gaussianRadius:    -1

    property string imageFile:         ""

    Component.onCompleted: {
        sketchEditor.helperImageReady.connect(helper.helperImageReady);
    }

    Keys.onReleased: {
        if (event.key === Qt.Key_Back) {
            if (sketchEditor.changed) {
                backMessageDialog.open();
            } else {
                mainStackView.pop();
            }

            event.accepted = true;
        }
    }

    onImageOrientationChanged: {
        if (imageOrientation !== -1 && gaussianRadius !== -1 && imageFile !== "") {
            sketchEditor.radius = gaussianRadius;

            sketchEditor.openImage(imageFile, imageOrientation);
        }
    }

    onGaussianRadiusChanged: {
        if (imageOrientation !== -1 && gaussianRadius !== -1 && imageFile !== "") {
            sketchEditor.radius = gaussianRadius;

            sketchEditor.openImage(imageFile, imageOrientation);
        }
    }

    onImageFileChanged: {
        if (imageOrientation !== -1 && gaussianRadius !== -1 && imageFile !== "") {
            sketchEditor.radius = gaussianRadius;

            sketchEditor.openImage(imageFile, imageOrientation);
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
                width:          UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                height:         UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                exclusiveGroup: buttonExclusiveGroup
                checkable:      true
                checked:        true
                enabled:        false

                style: ButtonStyle {
                    background: Rectangle {
                        implicitWidth:  control.width
                        implicitHeight: control.height
                        color:          control.checked ? "gray" : "lightgray"
                        radius:         UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 4)

                        Image {
                            anchors.fill:    parent
                            anchors.margins: UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 4)
                            source:          "images/mode_scroll.png"
                            fillMode:        Image.PreserveAspectFit
                        }
                    }
                }

                onCheckedChanged: {
                    if (checked) {
                        sketchEditor.mode           = SketchEditor.ModeScroll;
                        editorFlickable.interactive = true;
                        editorPinchArea.enabled     = true;
                    }
                }
            }

            Button {
                id:             originalModeButton
                width:          UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                height:         UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                exclusiveGroup: buttonExclusiveGroup
                checkable:      true
                enabled:        false

                style: ButtonStyle {
                    background: Rectangle {
                        implicitWidth:  control.width
                        implicitHeight: control.height
                        color:          control.checked ? "gray" : "lightgray"
                        radius:         UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 4)

                        Image {
                            anchors.fill:    parent
                            anchors.margins: UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 4)
                            source:          "images/mode_original.png"
                            fillMode:        Image.PreserveAspectFit
                        }
                    }
                }

                onCheckedChanged: {
                    if (checked) {
                        sketchEditor.mode           = SketchEditor.ModeOriginal;
                        editorFlickable.interactive = false;
                        editorPinchArea.enabled     = false;
                    }
                }
            }

            Button {
                id:             effectedModeButton
                width:          UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                height:         UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                exclusiveGroup: buttonExclusiveGroup
                checkable:      true
                enabled:        false

                style: ButtonStyle {
                    background: Rectangle {
                        implicitWidth:  control.width
                        implicitHeight: control.height
                        color:          control.checked ? "gray" : "lightgray"
                        radius:         UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 4)

                        Image {
                            anchors.fill:    parent
                            anchors.margins: UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 4)
                            source:          "images/mode_effected.png"
                            fillMode:        Image.PreserveAspectFit
                        }
                    }
                }

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

                SketchEditor {
                    id:              sketchEditor
                    scale:           editorFlickable.contentWidth        > 0.0 &&
                                     editorFlickable.initialContentWidth > 0.0 ?
                                     editorFlickable.contentWidth / editorFlickable.initialContentWidth : 1.0
                    transformOrigin: Item.TopLeft
                    helperSize:      helper.width
                    screenDPI:       AndroidGW.getScreenDPI()

                    onImageOpened: {
                        waitRectangle.visible = false;

                        saveToolButton.enabled  = true;
                        shareToolButton.enabled = true;

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

                        saveToolButton.enabled  = false;
                        shareToolButton.enabled = false;

                        scrollModeButton.enabled   = false;
                        originalModeButton.enabled = false;
                        effectedModeButton.enabled = false;

                        imageOpenFailedMessageDialog.open();
                    }

                    onImageSaved: {
                        AndroidGW.refreshGallery(image_file);

                        if (sketchPage.shareActionActive) {
                            AndroidGW.shareImage(image_file);
                        } else {
                            imageSavedMessageDialog.open();
                        }
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
            width:        UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 128)
            height:       UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 128)
            z:            5
            visible:      false
            color:        "black"
            border.color: "white"
            border.width: UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 2)

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
        height:         UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
        z:              1

        RowLayout {
            anchors.fill: parent

            ToolButton {
                width:  UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                height: UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)

                style: ButtonStyle {
                    background: Rectangle {
                        implicitWidth:  control.width
                        implicitHeight: control.height
                        color:          "transparent"

                        Image {
                            anchors.fill:    parent
                            anchors.margins: UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 4)
                            source:          "images/back.png"
                            fillMode:        Image.PreserveAspectFit
                        }
                    }
                }

                onClicked: {
                    if (sketchEditor.changed) {
                        backMessageDialog.open();
                    } else {
                        mainStackView.pop();
                    }
                }
            }

            ToolButton {
                id:      saveToolButton
                width:   UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                height:  UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                enabled: false

                style: ButtonStyle {
                    background: Rectangle {
                        implicitWidth:  control.width
                        implicitHeight: control.height
                        color:          "transparent"

                        Image {
                            anchors.fill:    parent
                            anchors.margins: UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 4)
                            source:          "images/save.png"
                            fillMode:        Image.PreserveAspectFit
                        }
                    }
                }

                onClicked: {
                    var date  = new Date();
                    var year  = date.getFullYear();
                    var month = date.getMonth() + 1;
                    var day   = date.getDate();
                    var hour  = date.getHours();
                    var min   = date.getMinutes();
                    var sec   = date.getSeconds();

                    var file_name = "IMG_" + year                              + "-" +
                                             (month > 9 ? month : "0" + month) + "-" +
                                             (day   > 9 ? day   : "0" + day)   + "_" +
                                             (hour  > 9 ? hour  : "0" + hour)  + "-" +
                                             (min   > 9 ? min   : "0" + min)   + "-" +
                                             (sec   > 9 ? sec   : "0" + sec)   + ".jpg";

                    sketchPage.shareActionActive = false;

                    sketchEditor.saveImage(AndroidGW.getSaveDirectory() + "/" + file_name);
                }
            }

            ToolButton {
                id:      shareToolButton
                width:   UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                height:  UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                enabled: false

                style: ButtonStyle {
                    background: Rectangle {
                        implicitWidth:  control.width
                        implicitHeight: control.height
                        color:          "transparent"

                        Image {
                            anchors.fill:    parent
                            anchors.margins: UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 4)
                            source:          "images/share.png"
                            fillMode:        Image.PreserveAspectFit
                        }
                    }
                }

                onClicked: {
                    var date  = new Date();
                    var year  = date.getFullYear();
                    var month = date.getMonth() + 1;
                    var day   = date.getDate();
                    var hour  = date.getHours();
                    var min   = date.getMinutes();
                    var sec   = date.getSeconds();

                    var file_name = "IMG_" + year                              + "-" +
                                             (month > 9 ? month : "0" + month) + "-" +
                                             (day   > 9 ? day   : "0" + day)   + "_" +
                                             (hour  > 9 ? hour  : "0" + hour)  + "-" +
                                             (min   > 9 ? min   : "0" + min)   + "-" +
                                             (sec   > 9 ? sec   : "0" + sec)   + ".jpg";

                    sketchPage.shareActionActive = true;

                    sketchEditor.saveImage(AndroidGW.getSaveDirectory() + "/" + file_name);
                }
            }

            ToolButton {
                id:      undoToolButton
                width:   UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                height:  UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                enabled: false

                style: ButtonStyle {
                    background: Rectangle {
                        implicitWidth:  control.width
                        implicitHeight: control.height
                        color:          "transparent"

                        Image {
                            anchors.fill:    parent
                            anchors.margins: UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 4)
                            source:          "images/undo.png"
                            fillMode:        Image.PreserveAspectFit
                        }
                    }
                }

                onClicked: {
                    sketchEditor.undo();
                }
            }

            ToolButton {
                width:  UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                height: UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)

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
    }

    MessageDialog {
        id:              imageOpenFailedMessageDialog
        title:           qsTr("Error")
        icon:            StandardIcon.Critical
        text:            qsTr("Could not open image")
        standardButtons: StandardButton.Ok
    }

    MessageDialog {
        id:              imageSavedMessageDialog
        title:           qsTr("Info")
        icon:            StandardIcon.Information
        text:            qsTr("Image saved successfully")
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
}
