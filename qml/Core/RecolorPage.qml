import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3
import ImageEditor 1.0

import "../Util.js" as UtilScript

Page {
    id: recolorPage

    header: Pane {
        topPadding:          recolorPage.bannerViewHeight
        Material.background: Material.LightBlue

        ButtonGroup {
            buttons: headerRow.children
        }

        Row {
            id:               headerRow
            anchors.centerIn: parent

            Button {
                id:        scrollModeButton
                width:     UtilScript.pt(48)
                height:    UtilScript.pt(48)
                enabled:   false
                checkable: true
                checked:   true

                contentItem: Image {
                    source:   "qrc:/resources/images/mode_scroll.png"
                    fillMode: Image.PreserveAspectFit
                }

                onCheckedChanged: {
                    if (checked) {
                        recolorEditor.mode          = RecolorEditor.ModeScroll;
                        editorFlickable.interactive = true;
                        editorPinchArea.enabled     = true;
                    }
                }
            }

            Button {
                id:        originalModeButton
                width:     UtilScript.pt(48)
                height:    UtilScript.pt(48)
                enabled:   false
                checkable: true

                contentItem: Image {
                    source:   "qrc:/resources/images/mode_original.png"
                    fillMode: Image.PreserveAspectFit
                }

                onCheckedChanged: {
                    if (checked) {
                        recolorEditor.mode          = RecolorEditor.ModeOriginal;
                        editorFlickable.interactive = false;
                        editorPinchArea.enabled     = false;
                    }
                }
            }

            Button {
                id:        effectedModeButton
                width:     UtilScript.pt(48)
                height:    UtilScript.pt(48)
                enabled:   false
                checkable: true

                contentItem: Image {
                    source:   "qrc:/resources/images/mode_effected.png"
                    fillMode: Image.PreserveAspectFit
                }

                onCheckedChanged: {
                    if (checked) {
                        recolorEditor.mode          = RecolorEditor.ModeEffected;
                        editorFlickable.interactive = false;
                        editorPinchArea.enabled     = false;
                    }
                }
            }

            Button {
                id:        hueSelectionModeButton
                width:     UtilScript.pt(48)
                height:    UtilScript.pt(48)
                enabled:   false
                checkable: true

                contentItem: Image {
                    source:   "qrc:/resources/images/mode_hue_selection.png"
                    fillMode: Image.PreserveAspectFit
                }

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

                        recolorPage.shareActionActive = false;

                        recolorEditor.saveImage(UIHelper.getSaveDirectory() + "/" + file_name);
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

                        recolorPage.shareActionActive = true;

                        recolorEditor.saveImage(UIHelper.getSaveDirectory() + "/" + file_name);
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
                    recolorEditor.undo();
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

    property bool   allowInterstitial: true
    property bool   shareActionActive: false

    property int    bannerViewHeight:  AdMobHelper.bannerViewHeight
    property int    imageOrientation:  -1

    property string imageFile:         ""

    Keys.onReleased: {
        if (event.key === Qt.Key_Back) {
            if (brushSettingsPane.visible) {
                brushSettingsPane.visible = false;
            } else if (recolorEditor.changed) {
                backMessageDialog.open();
            } else {
                mainStackView.pop();
            }

            event.accepted = true;
        }
    }

    onImageOrientationChanged: {
        if (imageOrientation !== -1 && imageFile !== "") {
            recolorEditor.openImage(imageFile, imageOrientation);
        }
    }

    onImageFileChanged: {
        if (imageOrientation !== -1 && imageFile !== "") {
            recolorEditor.openImage(imageFile, imageOrientation);
        }
    }

    function updateEditorParameters() {
        recolorEditor.brushSize    = AppSettings.brushSize;
        recolorEditor.brushOpacity = AppSettings.brushOpacity;
    }

    Rectangle {
        id:           editorRectangle
        anchors.fill: parent
        color:        "transparent"

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

                        saveToolButton.enabled  = true;
                        shareToolButton.enabled = true;

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

                        saveToolButton.enabled  = false;
                        shareToolButton.enabled = false;

                        scrollModeButton.enabled       = false;
                        originalModeButton.enabled     = false;
                        effectedModeButton.enabled     = false;
                        hueSelectionModeButton.enabled = false;

                        imageOpenFailedMessageDialog.open();
                    }

                    onImageSaved: {
                        UIHelper.refreshGallery(image_file);

                        if (recolorPage.shareActionActive) {
                            UIHelper.shareImage(image_file);
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
            width:        UtilScript.pt(128)
            height:       UtilScript.pt(128)
            z:            5
            visible:      false
            color:        "black"
            border.color: "white"
            border.width: UtilScript.pt(2)

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

            BusyIndicator {
                anchors.centerIn: parent
                running:          parent.visible
            }

            MouseArea {
                anchors.fill: parent
            }
        }
    }

    Rectangle {
        id:                     hueZoneRectangle
        anchors.right:          parent.right
        anchors.verticalCenter: parent.verticalCenter
        width:                  UtilScript.pt(48)
        height:                 parent.height * 3 / 4
        z:                      10
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

        onHeightChanged: {
            hueSliderRectangle.y = Math.max(0, Math.min(height - hueSliderRectangle.height, height - recolorEditor.hue / (359 / height)));
        }

        MouseArea {
            anchors.fill: parent

            Rectangle {
                id:            hueSliderRectangle
                anchors.left:  parent.left
                anchors.right: parent.right
                height:        UtilScript.pt(6)
                y:             (parent.height - height) * 0.5
                z:             11
                color:         "transparent"
                border.color:  "black"
                border.width:  UtilScript.pt(2)
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

    Pane {
        id:                 brushSettingsPane
        anchors.bottom:     parent.bottom
        anchors.left:       parent.left
        anchors.right:      parent.right
        z:                  15
        visible:            false
        Material.elevation: 5

        onVisibleChanged: {
            if (visible) {
                brushSizeSlider.value    = AppSettings.brushSize;
                brushOpacitySlider.value = AppSettings.brushOpacity;
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
                size:           brushSizeSlider.value
                maxSize:        brushSizeSlider.to
                opacity:        brushOpacitySlider.value
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

                        recolorPage.updateEditorParameters();
                    }
                }
            }

            Label {
                id:             brushSizeLabel
                text:           qsTr("Brush Size")
                font.pointSize: 16
            }

            Slider {
                id:               brushOpacitySlider
                from:             0.0
                to:               1.0
                value:            0.75
                stepSize:         0.1
                Layout.fillWidth: true

                onPressedChanged: {
                    if (!pressed) {
                        AppSettings.brushOpacity = value;

                        recolorPage.updateEditorParameters();
                    }
                }
            }

            Label {
                id:             brushOpacityLabel
                text:           qsTr("Brush Opacity")
                font.pointSize: 16
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

    Component.onCompleted: {
        recolorEditor.helperImageReady.connect(helper.helperImageReady);

        updateEditorParameters();
    }
}
