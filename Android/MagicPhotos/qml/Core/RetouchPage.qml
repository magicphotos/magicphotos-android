import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.1
import ImageEditor 1.0

import "Util.js" as UtilScript

Item {
    id: retouchPage

    property bool   shareActionActive: false

    property int    imageOrientation:  -1

    property string imageFile:         ""

    function updateEditorParameters() {
        retouchEditor.brushSize    = AppSettings.brushSize;
        retouchEditor.brushOpacity = AppSettings.brushOpacity;
    }

    Component.onCompleted: {
        retouchEditor.helperImageReady.connect(helper.helperImageReady);

        updateEditorParameters();
    }

    Keys.onReleased: {
        if (event.key === Qt.Key_Back) {
            if (brushSettingsRectangle.visible) {
                brushSettingsRectangle.visible = false;
            } else if (retouchEditor.changed) {
                backMessageDialog.open();
            } else {
                mainStackView.pop();
            }

            event.accepted = true;
        }
    }

    onImageOrientationChanged: {
        if (imageOrientation !== -1 && imageFile !== "") {
            retouchEditor.openImage(imageFile, imageOrientation);
        }
    }

    onImageFileChanged: {
        if (imageOrientation !== -1 && imageFile !== "") {
            retouchEditor.openImage(imageFile, imageOrientation);
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
                        retouchEditor.mode          = RetouchEditor.ModeScroll;
                        editorFlickable.interactive = true;
                        editorPinchArea.enabled     = true;
                    }
                }
            }

            Button {
                id:             samplingPointModeButton
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
                        id:              retouchEditor
                        scale:           editorFlickable.contentWidth        > 0.0 &&
                                         editorFlickable.initialContentWidth > 0.0 ?
                                         editorFlickable.contentWidth / editorFlickable.initialContentWidth : 1.0
                        transformOrigin: Item.TopLeft
                        helperSize:      helper.width

                        onImageOpened: {
                            waitRectangle.visible = false;

                            saveToolButton.enabled  = true;
                            shareToolButton.enabled = true;

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

                            saveToolButton.enabled  = false;
                            shareToolButton.enabled = false;

                            scrollModeButton.enabled        = false;
                            samplingPointModeButton.enabled = false;
                            cloneModeButton.enabled         = false;
                            blurModeButton.enabled          = false;

                            imageOpenFailedMessageDialog.open();
                        }

                        onImageSaved: {
                            AndroidGW.refreshGallery(image_file);

                            if (retouchPage.shareActionActive) {
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
                        width:   UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                        height:  UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
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
                    width:            UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                    height:           UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 48)
                    source:           "images/busy_indicator.png"
                    fillMode:         Image.PreserveAspectFit
                }
            }
        }
    }

    Rectangle {
        id:             brushSettingsRectangle
        anchors.bottom: bottomToolBar.top
        anchors.left:   parent.left
        anchors.right:  parent.right
        height:         Math.max(brushSizeRectangle.height    + brushSizeRectangle.anchors.bottomMargin    +
                                 brushOpacityRectangle.height + brushOpacityRectangle.anchors.bottomMargin + UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 12), brushPreviewGenerator.height)
        z:              15
        color:          "black"
        visible:        false

        onVisibleChanged: {
            if (visible) {
                brushSizeSlider.value    = AppSettings.brushSize;
                brushOpacitySlider.value = AppSettings.brushOpacity;
            }
        }

        MouseArea {
            anchors.fill: parent

            Rectangle {
                id:                   brushSizeRectangle
                anchors.bottom:       brushOpacityRectangle.top
                anchors.left:         parent.left
                anchors.right:        brushPreviewGenerator.left
                anchors.bottomMargin: UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 12)
                anchors.rightMargin:  UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 12)
                height:               brushSizeTextRectangle.height
                color:                "transparent"

                Rectangle {
                    id:           brushSizeTextRectangle
                    anchors.left: parent.left
                    width:        Math.max(brushSizeText.width, brushOpacityText.width)
                    height:       Math.max(brushSizeText.height, brushSizeSlider.height)
                    color:        "transparent"

                    Text {
                        id:                     brushSizeText
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left:           parent.left
                        color:                  "white"
                        text:                   qsTr("Brush Size")
                    }
                }

                Slider {
                    id:                     brushSizeSlider
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left:           brushSizeTextRectangle.right
                    anchors.right:          parent.right
                    anchors.leftMargin:     UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 12)
                    minimumValue:           UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 8)
                    maximumValue:           UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 24)
                    value:                  UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 16)
                    stepSize:               1.0

                    onPressedChanged: {
                        if (!pressed) {
                            AppSettings.brushSize = value;

                            retouchPage.updateEditorParameters();
                        }
                    }
                }
            }

            Rectangle {
                id:                   brushOpacityRectangle
                anchors.bottom:       parent.bottom
                anchors.left:         parent.left
                anchors.right:        brushPreviewGenerator.left
                anchors.bottomMargin: UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 12)
                anchors.rightMargin:  UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 12)
                height:               brushOpacityTextRectangle.height
                color:                "transparent"

                Rectangle {
                    id:                     brushOpacityTextRectangle
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left:           parent.left
                    width:                  Math.max(brushSizeText.width, brushOpacityText.width)
                    height:                 Math.max(brushOpacityText.height, brushOpacitySlider.height)
                    color:                  "transparent"

                    Text {
                        id:                     brushOpacityText
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left:           parent.left
                        color:                  "white"
                        text:                   qsTr("Brush Opacity")
                    }
                }

                Slider {
                    id:                     brushOpacitySlider
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left:           brushOpacityTextRectangle.right
                    anchors.right:          parent.right
                    anchors.leftMargin:     UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 12)
                    minimumValue:           0.0
                    maximumValue:           1.0
                    value:                  0.75
                    stepSize:               0.1

                    onPressedChanged: {
                        if (!pressed) {
                            AppSettings.brushOpacity = value;

                            retouchPage.updateEditorParameters();
                        }
                    }
                }
            }

            BrushPreviewGenerator {
                id:                     brushPreviewGenerator
                anchors.verticalCenter: parent.verticalCenter
                anchors.right:          parent.right
                size:                   brushSizeSlider.value
                maxSize:                brushSizeSlider.maximumValue
                opacity:                brushOpacitySlider.value
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

                MouseArea {
                    anchors.fill: parent
                }
            }
        }

        RowLayout {
            anchors.fill: parent

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
                    if (AndroidGW.getFullVersion()) {
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

                        retouchPage.shareActionActive = false;

                        retouchEditor.saveImage(AndroidGW.getSaveDirectory() + "/" + file_name);
                    } else {
                        purchaseMessageDialog.open();
                    }
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
                    if (AndroidGW.getFullVersion()) {
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

                        retouchPage.shareActionActive = true;

                        retouchEditor.saveImage(AndroidGW.getSaveDirectory() + "/" + file_name);
                    } else {
                        purchaseMessageDialog.open();
                    }
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
                    retouchEditor.undo();
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
                            source:          "images/settings.png"
                            fillMode:        Image.PreserveAspectFit
                        }
                    }
                }

                onClicked: {
                    brushSettingsRectangle.visible = !brushSettingsRectangle.visible;
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

    MessageDialog {
        id:              purchaseMessageDialog
        title:           qsTr("Warning")
        icon:            StandardIcon.Warning
        text:            qsTr("This function is available in the full version only. Do you want to purchase full version now?")
        standardButtons: StandardButton.Yes | StandardButton.No

        onYes: {
            if (!AndroidGW.buyFullVersion()) {
                purchaseFailedMessageDialog.open();
            }
        }
    }

    MessageDialog {
        id:              purchaseFailedMessageDialog
        title:           qsTr("Error")
        icon:            StandardIcon.Critical
        text:            qsTr("Purchase attempt failed, in-app billing may be not supported on this device")
        standardButtons: StandardButton.Ok
    }
}
