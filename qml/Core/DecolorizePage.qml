import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.3
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import ImageEditor 1.0

import "../Util.js" as UtilScript

Page {
    id: decolorizePage

    header: Pane {
        topPadding:          decolorizePage.bannerViewHeight
        Material.background: Material.LightBlue

        ButtonGroup {
            buttons: headerRow.children
        }

        Row {
            id:               headerRow
            anchors.centerIn: parent

            Button {
                id:             scrollModeButton
                implicitWidth:  UtilScript.dp(UIHelper.screenDpi, 48)
                implicitHeight: UtilScript.dp(UIHelper.screenDpi, 48)
                enabled:        false
                checkable:      true
                checked:        true

                contentItem: Image {
                    source:   "qrc:/resources/images/mode_scroll.png"
                    fillMode: Image.PreserveAspectFit
                }
            }

            Button {
                id:             originalModeButton
                implicitWidth:  UtilScript.dp(UIHelper.screenDpi, 48)
                implicitHeight: UtilScript.dp(UIHelper.screenDpi, 48)
                enabled:        false
                checkable:      true

                contentItem: Image {
                    source:   "qrc:/resources/images/mode_original.png"
                    fillMode: Image.PreserveAspectFit
                }
            }

            Button {
                id:             effectedModeButton
                implicitWidth:  UtilScript.dp(UIHelper.screenDpi, 48)
                implicitHeight: UtilScript.dp(UIHelper.screenDpi, 48)
                enabled:        false
                checkable:      true

                contentItem: Image {
                    source:   "qrc:/resources/images/mode_effected.png"
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
                    if (UIHelper.requestWriteStoragePermission()) {
                        decolorizePage.shareActionActive = false;

                        decolorizeEditor.saveImage(UtilScript.generateImageFileName(UIHelper.getSaveDirectory()));
                    }
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
                    if (UIHelper.requestWriteStoragePermission()) {
                        decolorizePage.shareActionActive = true;

                        decolorizeEditor.saveImage(UtilScript.generateImageFileName(UIHelper.getSaveDirectory()));
                    }
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
                    decolorizeEditor.undo();
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

    readonly property int bannerViewHeight:   AdMobHelper.bannerViewHeight

    property bool shareActionActive:          false

    property int imageOrientation:            -1

    property string imageFile:                ""

    Keys.onReleased: {
        if (event.key === Qt.Key_Back) {
            if (brushSettingsPane.visible) {
                brushSettingsPane.visible = false;
            } else if (decolorizeEditor.changed) {
                backMessageDialog.open();
            } else {
                mainStackView.pop();
            }

            event.accepted = true;
        }
    }

    onImageOrientationChanged: {
        if (imageOrientation !== -1 && imageFile !== "") {
            decolorizeEditor.openImage(imageFile, imageOrientation);
        }
    }

    onImageFileChanged: {
        if (imageOrientation !== -1 && imageFile !== "") {
            decolorizeEditor.openImage(imageFile, imageOrientation);
        }
    }

    function updateEditorParameters() {
        decolorizeEditor.brushSize     = AppSettings.brushSize;
        decolorizeEditor.brushHardness = AppSettings.brushHardness;
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

                DecolorizeEditor {
                    id:              decolorizeEditor
                    scale:           editorFlickable.contentWidth        > 0.0 &&
                                     editorFlickable.initialContentWidth > 0.0 ?
                                     editorFlickable.contentWidth / editorFlickable.initialContentWidth : 1.0
                    transformOrigin: Item.TopLeft
                    mode:            editorMode(scrollModeButton.checked, originalModeButton.checked, effectedModeButton.checked)
                    helperSize:      helper.width

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
                        UIHelper.refreshGallery(imageFile);

                        if (decolorizePage.shareActionActive) {
                            UIHelper.shareImage(imageFile);
                        } else {
                            imageSavedMessageDialog.open();
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
                        var rect = mapToItem(editorRectangle, x, y);

                        if (eventType === DecolorizeEditor.MousePressed) {
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
                        } else if (eventType === DecolorizeEditor.MouseMoved) {
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
                        } else if (eventType === DecolorizeEditor.MouseReleased) {
                            helperRectangle.visible = false;
                        }
                    }

                    onHelperImageReady: {
                        helper.setHelperImage(helperImage);
                    }

                    function editorMode(scroll_mode, original_mode, effected_mode) {
                        if (scroll_mode) {
                            return DecolorizeEditor.ModeScroll;
                        } else if (original_mode) {
                            return DecolorizeEditor.ModeOriginal;
                        } else if (effected_mode) {
                            return DecolorizeEditor.ModeEffected;
                        } else {
                            return DecolorizeEditor.ModeScroll;
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

                        decolorizePage.updateEditorParameters();
                    }
                }
            }

            Label {
                id:             brushSizeLabel
                text:           qsTr("Brush Size")
                font.pixelSize: UtilScript.dp(UIHelper.screenDpi, 16)
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

                        decolorizePage.updateEditorParameters();
                    }
                }
            }

            Label {
                id:             brushHardnessLabel
                text:           qsTr("Brush Hardness")
                font.pixelSize: UtilScript.dp(UIHelper.screenDpi, 16)
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
