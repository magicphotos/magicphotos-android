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
        Material.background: Material.LightBlue

        ButtonGroup {
            buttons: headerRow.children
        }

        Row {
            id:               headerRow
            anchors.centerIn: parent
            enabled:          false

            Button {
                id:             scrollModeButton
                implicitWidth:  UtilScript.dp(UIHelper.screenDpi, 48)
                implicitHeight: UtilScript.dp(UIHelper.screenDpi, 48)
                checkable:      true
                checked:        true

                contentItem: Image {
                    source:   "qrc:/resources/images/mode_scroll.png"
                    fillMode: Image.PreserveAspectFit
                }
            }

            Button {
                id:             samplingPointModeButton
                implicitWidth:  UtilScript.dp(UIHelper.screenDpi, 48)
                implicitHeight: UtilScript.dp(UIHelper.screenDpi, 48)
                checkable:      true

                contentItem: Image {
                    source:   "qrc:/resources/images/mode_sampling_point.png"
                    fillMode: Image.PreserveAspectFit
                }
            }

            Button {
                id:             cloneModeButton
                implicitWidth:  UtilScript.dp(UIHelper.screenDpi, 48)
                implicitHeight: UtilScript.dp(UIHelper.screenDpi, 48)
                checkable:      true

                contentItem: Image {
                    source:   "qrc:/resources/images/mode_clone.png"
                    fillMode: Image.PreserveAspectFit
                }
            }

            Button {
                id:             blurModeButton
                implicitWidth:  UtilScript.dp(UIHelper.screenDpi, 48)
                implicitHeight: UtilScript.dp(UIHelper.screenDpi, 48)
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
                implicitWidth:    UtilScript.dp(UIHelper.screenDpi, 48)
                implicitHeight:   UtilScript.dp(UIHelper.screenDpi, 48)
                enabled:          false
                Layout.alignment: Qt.AlignHCenter

                contentItem: Image {
                    source:   "qrc:/resources/images/tool_save.png"
                    fillMode: Image.PreserveAspectFit
                }

                onClicked: {
                    retouchPage.shareActionActive = false;

                    retouchPage.editor.saveImage(MediaStoreHelper.imageFilePath);
                }
            }

            ToolButton {
                id:               shareToolButton
                implicitWidth:    UtilScript.dp(UIHelper.screenDpi, 48)
                implicitHeight:   UtilScript.dp(UIHelper.screenDpi, 48)
                enabled:          false
                Layout.alignment: Qt.AlignHCenter

                contentItem: Image {
                    source:   "qrc:/resources/images/tool_share.png"
                    fillMode: Image.PreserveAspectFit
                }

                onClicked: {
                    retouchPage.shareActionActive = true;

                    retouchPage.editor.saveImage(MediaStoreHelper.imageFilePath);
                }
            }

            ToolButton {
                id:               undoToolButton
                implicitWidth:    UtilScript.dp(UIHelper.screenDpi, 48)
                implicitHeight:   UtilScript.dp(UIHelper.screenDpi, 48)
                enabled:          false
                Layout.alignment: Qt.AlignHCenter

                contentItem: Image {
                    source:   "qrc:/resources/images/tool_undo.png"
                    fillMode: Image.PreserveAspectFit
                }

                onClicked: {
                    retouchPage.editor.undo();
                }
            }

            ToolButton {
                implicitWidth:    UtilScript.dp(UIHelper.screenDpi, 48)
                implicitHeight:   UtilScript.dp(UIHelper.screenDpi, 48)
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
                implicitWidth:    UtilScript.dp(UIHelper.screenDpi, 48)
                implicitHeight:   UtilScript.dp(UIHelper.screenDpi, 48)
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

    readonly property var editor:             editorLoader.item

    property bool shareActionActive:          false

    property int imageOrientation:            -1

    property string imagePath:                ""

    property var editorComponent: Component {
        RetouchEditor {
        }
    }

    Keys.onReleased: {
        if (event.key === Qt.Key_Back) {
            if (brushSettingsPane.visible) {
                brushSettingsPane.visible = false;
            } else if (editor.changed) {
                backMessageDialog.open();
            } else {
                mainStackView.pop();
            }

            event.accepted = true;
        }
    }

    function updateEditorParameters() {
        editor.brushSize     = AppSettings.brushSize;
        editor.brushHardness = AppSettings.brushHardness;
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
                    width:  retouchPage.editor.width  * retouchPage.editor.scale
                    height: retouchPage.editor.height * retouchPage.editor.scale
                    color:  "transparent"
                    clip:   true

                    Loader {
                        id:              editorLoader
                        sourceComponent: retouchPage.editorComponent

                        onLoaded: {
                            item.scale           = Qt.binding(function() { return editorFlickable.contentWidth > 0.0 && editorFlickable.initialContentWidth > 0.0 ? editorFlickable.contentWidth / editorFlickable.initialContentWidth : 1.0; });
                            item.transformOrigin = Item.TopLeft;
                            item.mode            = Qt.binding(function() { return editorMode(scrollModeButton.checked, samplingPointModeButton.checked, cloneModeButton.checked, blurModeButton.checked); });
                            item.helperSize      = Qt.binding(function() { return helper.width; });

                            retouchPage.updateEditorParameters();
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
                        x:       imageX(width,  retouchPage.editor.samplingPoint.x, editorFlickable.contentWidth, editorFlickable.initialContentWidth)
                        y:       imageY(height, retouchPage.editor.samplingPoint.y, editorFlickable.contentWidth, editorFlickable.initialContentWidth)
                        width:   UtilScript.dp(UIHelper.screenDpi, 48)
                        height:  UtilScript.dp(UIHelper.screenDpi, 48)
                        source:  "qrc:/resources/images/sampling_point.png"
                        visible: retouchPage.editor.samplingPointValid

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
            width:        UtilScript.dp(UIHelper.screenDpi, 128)
            height:       UtilScript.dp(UIHelper.screenDpi, 128)
            color:        "transparent"
            visible:      false

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
            anchors.margins: UtilScript.dp(UIHelper.screenDpi, 4)
            rows:            2
            columns:         3
            rowSpacing:      UtilScript.dp(UIHelper.screenDpi, 8)
            columnSpacing:   UtilScript.dp(UIHelper.screenDpi, 4)

            BrushPreviewGenerator {
                id:             brushPreviewGenerator
                brushSize:      brushSizeSlider.value
                maxBrushSize:   brushSizeSlider.to
                brushHardness:  brushHardnessSlider.value
                Layout.rowSpan: 2
            }

            Slider {
                id:               brushSizeSlider
                from:             UtilScript.dp(UIHelper.screenDpi, 8)
                to:               UtilScript.dp(UIHelper.screenDpi, 24)
                value:            UtilScript.dp(UIHelper.screenDpi, 16)
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
                font.pixelSize: UtilScript.dp(UIHelper.screenDpi, 16)
                font.family:    "Helvetica"
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
                font.pixelSize: UtilScript.dp(UIHelper.screenDpi, 16)
                font.family:    "Helvetica"
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

        onAccepted: {
            if (Math.random() < 0.10) {
                GPlayHelper.requestReview();
            }
        }
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

    Connections {
        target: retouchPage.editor

        onImageOpened: {
            waitRectangle.visible = false;

            headerRow.enabled       = true;
            saveToolButton.enabled  = true;
            shareToolButton.enabled = true;

            editorFlickable.contentWidth         = retouchPage.editor.width;
            editorFlickable.contentHeight        = retouchPage.editor.height;
            editorFlickable.initialContentWidth  = retouchPage.editor.width;
            editorFlickable.initialContentHeight = retouchPage.editor.height;
        }

        onImageOpenFailed: {
            waitRectangle.visible = false;

            headerRow.enabled       = false;
            saveToolButton.enabled  = false;
            shareToolButton.enabled = false;

            imageOpenFailedMessageDialog.open();
        }

        onImageSaved: {
            if (retouchPage.shareActionActive) {
                UIHelper.shareImage(imagePath);
            } else if (UIHelper.requestWriteStoragePermission() &&
                       MediaStoreHelper.addImageToMediaStore(imagePath)) {
                imageSavedMessageDialog.open();
            } else {
                imageSaveFailedMessageDialog.open();
            }
        }

        onImageSaveFailed: {
            imageSaveFailedMessageDialog.open();
        }

        onUndoAvailabilityUpdated: {
            if (available) {
                undoToolButton.enabled = true;
            } else {
                undoToolButton.enabled = false;
            }
        }

        onMouseEvent: {
            var rect = retouchPage.editor.mapToItem(editorRectangle, x, y);

            if (eventType === Editor.MousePressed ||
                eventType === Editor.MouseMoved) {
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
            } else if (eventType === Editor.MouseReleased) {
                helperRectangle.visible = false;
            }
        }

        onHelperImageReady: {
            helper.setHelperImage(helperImage);
        }
    }

    Component.onCompleted: {
        if (imageOrientation !== -1 && imagePath !== "") {
            editor.openImage(imagePath, imageOrientation);
        }
    }
}
