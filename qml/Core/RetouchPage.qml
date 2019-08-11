import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.3
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import ImageEditor 1.0

import "../Util.js" as UtilScript

Page {
    id: retouchPage

    header: Pane {
        topPadding:          retouchPage.bannerViewHeight
        Material.background: Material.LightBlue

        ButtonGroup {
            buttons: headerRow.children
        }

        Row {
            id:               headerRow
            anchors.centerIn: parent

            Button {
                id:             scrollModeButton
                implicitWidth:  UtilScript.pt(48)
                implicitHeight: UtilScript.pt(48)
                enabled:        false
                checkable:      true
                checked:        true

                contentItem: Image {
                    source:   "qrc:/resources/images/mode_scroll.png"
                    fillMode: Image.PreserveAspectFit
                }
            }

            Button {
                id:             samplingPointModeButton
                implicitWidth:  UtilScript.pt(48)
                implicitHeight: UtilScript.pt(48)
                enabled:        false
                checkable:      true

                contentItem: Image {
                    source:   "qrc:/resources/images/mode_sampling_point.png"
                    fillMode: Image.PreserveAspectFit
                }
            }

            Button {
                id:             cloneModeButton
                implicitWidth:  UtilScript.pt(48)
                implicitHeight: UtilScript.pt(48)
                enabled:        false
                checkable:      true

                contentItem: Image {
                    source:   "qrc:/resources/images/mode_clone.png"
                    fillMode: Image.PreserveAspectFit
                }
            }

            Button {
                id:             blurModeButton
                implicitWidth:  UtilScript.pt(48)
                implicitHeight: UtilScript.pt(48)
                enabled:        false
                checkable:      true

                contentItem: Image {
                    source:   "qrc:/resources/images/mode_blur.png"
                    fillMode: Image.PreserveAspectFit
                }
            }
        }
    }

    footer: ToolBar {
        RowLayout {
            anchors.fill: parent

            ToolButton {
                id:               saveToolButton
                implicitWidth:    UtilScript.pt(48)
                implicitHeight:   UtilScript.pt(48)
                enabled:          false
                Layout.alignment: Qt.AlignHCenter

                contentItem: Image {
                    source:   "qrc:/resources/images/tool_save.png"
                    fillMode: Image.PreserveAspectFit
                }

                onClicked: {
                    if (UIHelper.requestWriteStoragePermission()) {
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

                        retouchEditor.saveImage(UIHelper.getSaveDirectory() + "/" + file_name);
                    }
                }
            }

            ToolButton {
                id:               shareToolButton
                implicitWidth:    UtilScript.pt(48)
                implicitHeight:   UtilScript.pt(48)
                enabled:          false
                Layout.alignment: Qt.AlignHCenter

                contentItem: Image {
                    source:   "qrc:/resources/images/tool_share.png"
                    fillMode: Image.PreserveAspectFit
                }

                onClicked: {
                    if (UIHelper.requestWriteStoragePermission()) {
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

                        retouchEditor.saveImage(UIHelper.getSaveDirectory() + "/" + file_name);
                    }
                }
            }

            ToolButton {
                id:               undoToolButton
                implicitWidth:    UtilScript.pt(48)
                implicitHeight:   UtilScript.pt(48)
                enabled:          false
                Layout.alignment: Qt.AlignHCenter

                contentItem: Image {
                    source:   "qrc:/resources/images/tool_undo.png"
                    fillMode: Image.PreserveAspectFit
                }

                onClicked: {
                    retouchEditor.undo();
                }
            }

            ToolButton {
                implicitWidth:    UtilScript.pt(48)
                implicitHeight:   UtilScript.pt(48)
                Layout.alignment: Qt.AlignHCenter

                contentItem: Image {
                    source:   "qrc:/resources/images/tool_settings.png"
                    fillMode: Image.PreserveAspectFit
                }

                onClicked: {
                    brushSettingsPane.visible = !brushSettingsPane.visible;
                }
            }

            ToolButton {
                implicitWidth:    UtilScript.pt(48)
                implicitHeight:   UtilScript.pt(48)
                Layout.alignment: Qt.AlignHCenter

                contentItem: Image {
                    source:   "qrc:/resources/images/tool_help.png"
                    fillMode: Image.PreserveAspectFit
                }

                onClicked: {
                    Qt.openUrlExternally(qsTr("https://magicphotos.sourceforge.io/help/android/help.html"));
                }
            }
        }
    }

    readonly property bool allowInterstitial: true

    readonly property int bannerViewHeight:   AdMobHelper.bannerViewHeight

    property bool shareActionActive:          false

    property int imageOrientation:            -1

    property string imageFile:                ""

    Keys.onReleased: {
        if (event.key === Qt.Key_Back) {
            if (brushSettingsPane.visible) {
                brushSettingsPane.visible = false;
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

    function updateEditorParameters() {
        retouchEditor.brushSize     = AppSettings.brushSize;
        retouchEditor.brushHardness = AppSettings.brushHardness;
    }

    Rectangle {
        id:           editorRectangle
        anchors.fill: parent
        color:        "transparent"

        PinchArea {
            id:           editorPinchArea
            anchors.fill: parent
            enabled:      scrollModeButton.checked

            onPinchUpdated: {
                var pinch_prev_center = mapToItem(editorFlickable.contentItem, pinch.previousCenter.x, pinch.previousCenter.y);
                var pinch_center      = mapToItem(editorFlickable.contentItem, pinch.center.x, pinch.center.y);
                var pinch_prev_scale  = pinch.previousScale;
                var pinch_scale       = pinch.scale;

                if (editorFlickable.initialContentWidth > 0.0) {
                    editorFlickable.contentX += pinch_prev_center.x - pinch_center.x;
                    editorFlickable.contentY += pinch_prev_center.y - pinch_center.y;

                    var scale = 1.0 + pinch_scale - pinch_prev_scale;

                    if (editorFlickable.contentWidth * scale / editorFlickable.initialContentWidth >= 0.5 &&
                        editorFlickable.contentWidth * scale / editorFlickable.initialContentWidth <= 4.0) {
                        editorFlickable.resizeContent(editorFlickable.contentWidth * scale, editorFlickable.contentHeight * scale, pinch_center);
                    }
                }
            }

            onPinchFinished: {
                editorFlickable.returnToBounds();
            }

            Flickable {
                id:             editorFlickable
                anchors.fill:   parent
                leftMargin:     width  > contentWidth  ? (width  - contentWidth)  / 2 : 0
                topMargin:      height > contentHeight ? (height - contentHeight) / 2 : 0
                boundsBehavior: Flickable.StopAtBounds
                interactive:    scrollModeButton.checked

                property real initialContentWidth:  0.0
                property real initialContentHeight: 0.0

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
                        mode:            editorMode(scrollModeButton.checked, samplingPointModeButton.checked, cloneModeButton.checked, blurModeButton.checked)
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
                            UIHelper.refreshGallery(imageFile);

                            if (retouchPage.shareActionActive) {
                                UIHelper.shareImage(imageFile);
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

                            if (eventType === RetouchEditor.MousePressed) {
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
                            } else if (eventType === RetouchEditor.MouseMoved) {
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
                            } else if (eventType === RetouchEditor.MouseReleased) {
                                helperRectangle.visible = false;
                            }
                        }

                        onHelperImageReady: {
                            helper.helperImageReady(helperImage);
                        }

                        function editorMode(scroll_mode, sampling_point_mode, clone_mode, blur_mode) {
                            if (scroll_mode) {
                                return RetouchEditor.ModeScroll;
                            } else if (sampling_point_mode) {
                                return RetouchEditor.ModeSamplingPoint;
                            } else if (clone_mode) {
                                return RetouchEditor.ModeClone;
                            } else if (blur_mode) {
                                return RetouchEditor.ModeBlur;
                            } else {
                                return RetouchEditor.ModeScroll;
                            }
                        }
                    }

                    Image {
                        id:      samplingPointImage
                        x:       imageX(width,  retouchEditor.samplingPoint.x, editorFlickable.contentWidth, editorFlickable.initialContentWidth)
                        y:       imageY(height, retouchEditor.samplingPoint.y, editorFlickable.contentWidth, editorFlickable.initialContentWidth)
                        width:   UtilScript.pt(48)
                        height:  UtilScript.pt(48)
                        source:  "qrc:/resources/images/sampling_point.png"
                        visible: retouchEditor.samplingPointValid

                        function imageX(width, sampling_point_x, content_width, initial_content_width) {
                            if (content_width > 0.0 && initial_content_width > 0.0) {
                                return sampling_point_x * content_width / initial_content_width - width / 2;
                            } else {
                                return 0;
                            }
                        }

                        function imageY(height, sampling_point_y, content_width, initial_content_width) {
                            if (content_width > 0.0 && initial_content_width > 0.0) {
                                return sampling_point_y * content_width / initial_content_width - height / 2;
                            } else {
                                return 0;
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
            z:            1
            width:        UtilScript.pt(128)
            height:       UtilScript.pt(128)
            visible:      false
            color:        "transparent"

            OpacityMask {
                id:           opacityMask
                anchors.fill: parent

                source: Helper {
                    id:      helper
                    width:   opacityMask.width
                    height:  opacityMask.height
                    visible: false
                }

                maskSource: Image {
                    width:    opacityMask.width
                    height:   opacityMask.height
                    source:   "qrc:/resources/images/helper_mask.png"
                    fillMode: Image.PreserveAspectFit
                    visible:  false
                }
            }
        }

        Rectangle {
            id:           waitRectangle
            anchors.fill: parent
            z:            2
            color:        "black"
            opacity:      0.75

            BusyIndicator {
                anchors.centerIn: parent
                running:          parent.visible
            }

            MultiPointTouchArea {
                anchors.fill: parent
            }
        }
    }

    Pane {
        id:                 brushSettingsPane
        anchors.bottom:     parent.bottom
        anchors.left:       parent.left
        anchors.right:      parent.right
        z:                  1
        visible:            false
        Material.elevation: 5

        onVisibleChanged: {
            if (visible) {
                brushSizeSlider.value     = AppSettings.brushSize;
                brushHardnessSlider.value = AppSettings.brushHardness;
            }
        }

        GridLayout {
            anchors.fill:    parent
            anchors.margins: UtilScript.pt(4)
            rows:            2
            columns:         3
            rowSpacing:      UtilScript.pt(8)
            columnSpacing:   UtilScript.pt(4)

            BrushPreviewGenerator {
                id:             brushPreviewGenerator
                brushSize:      brushSizeSlider.value
                maxBrushSize:   brushSizeSlider.to
                brushHardness:  brushHardnessSlider.value
                Layout.rowSpan: 2
            }

            Slider {
                id:               brushSizeSlider
                from:             UtilScript.pt(8)
                to:               UtilScript.pt(24)
                value:            UtilScript.pt(16)
                stepSize:         1.0
                Layout.fillWidth: true

                onPressedChanged: {
                    if (!pressed) {
                        AppSettings.brushSize = value;

                        retouchPage.updateEditorParameters();
                    }
                }
            }

            Label {
                id:             brushSizeLabel
                text:           qsTr("Brush Size")
                font.pointSize: 16
            }

            Slider {
                id:               brushHardnessSlider
                from:             0.0
                to:               1.0
                value:            0.75
                stepSize:         0.1
                Layout.fillWidth: true

                onPressedChanged: {
                    if (!pressed) {
                        AppSettings.brushHardness = value;

                        retouchPage.updateEditorParameters();
                    }
                }
            }

            Label {
                id:             brushHardnessLabel
                text:           qsTr("Brush Hardness")
                font.pointSize: 16
            }
        }
    }

    MessageDialog {
        id:              imageOpenFailedMessageDialog
        title:           qsTr("Error")
        text:            qsTr("Could not open image")
        standardButtons: StandardButton.Ok
    }

    MessageDialog {
        id:              imageSavedMessageDialog
        title:           qsTr("Info")
        text:            qsTr("Image saved successfully")
        standardButtons: StandardButton.Ok
    }

    MessageDialog {
        id:              imageSaveFailedMessageDialog
        title:           qsTr("Error")
        text:            qsTr("Could not save image")
        standardButtons: StandardButton.Ok
    }

    MessageDialog {
        id:              backMessageDialog
        title:           qsTr("Warning")
        text:            qsTr("Are you sure? Current image is not saved and will be lost.")
        standardButtons: StandardButton.Yes | StandardButton.No

        onYes: {
            mainStackView.pop();
        }
    }

    Component.onCompleted: {
        updateEditorParameters();
    }
}
