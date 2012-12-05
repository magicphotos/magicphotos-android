import QtQuick 1.0
import com.nokia.symbian 1.0

import PaintItem 1.0

import "PaintPage.js" as PaintPageScript

Page {
    id:           paintPage
    anchors.fill: parent

    property int buttonWidth:             52
    property int scalePercentTextWidth:   64
    property int scaleZoneHeight:         64
    property int hueZoneWidth:            48
    property int hueZoneHeight:           240
    property int hueSliderHeight:         6

    property int currentEffect:           PaintPageScript.EFFECT_GRAYSCALE

    property bool imageWasChanged:        false
    property bool imageOpenRequested:     false
    property bool quitRequested:          false

    property string exampleImageResource: ":/resources/images/examples/example.png"

    function offerExampleImage() {
        waitRectangle.visible = true;

        offerExampleImageTimer.start();
    }

    function openImage(imageUrl) {
        waitRectangle.visible = true;

        paintItem.openImage(imageUrl);
    }

    function saveImage(imageUrl) {
        paintItem.saveImage(imageUrl);
    }

    function saveCancelled() {
        imageOpenRequested = false;
        quitRequested      = false;
    }

    Rectangle {
        id:            topButtonGroupRectangle
        anchors.top:   parent.top
        anchors.left:  parent.left
        anchors.right: parent.right
        height:        Math.max(Math.max(undoButton.height, modeButtonRow.height), scaleButton.height)
        z:             1
        color:         "transparent"

        Button {
            id:           undoButton
            anchors.top:  parent.top
            anchors.left: parent.left
            width:        paintPage.buttonWidth
            iconSource:   "qrc:/resources/images/undo.png"
            enabled:      false

            onClicked: {
                paintItem.undo();
            }
        }

        ButtonRow {
            id:                       modeButtonRow
            anchors.top:              parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            width:                    huePickerButton.visible ? paintPage.buttonWidth * 4 : paintPage.buttonWidth * 3
            exclusive:                true
            checkedButton:            scrollModeButton

            Button {
                id:         scrollModeButton
                iconSource: "qrc:/resources/images/mode_scroll.png"
                enabled:    false

                onCheckedChanged: {
                    if (checked) {
                        paintItem.setMode(PaintPageScript.MODE_SCROLL);

                        paintItemFlickable.interactive = true;
                    }
                }
            }

            Button {
                id:         originalModeButton
                iconSource: "qrc:/resources/images/mode_original.png"
                enabled:    false

                onCheckedChanged: {
                    if (checked) {
                        paintItem.setMode(PaintPageScript.MODE_ORIGINAL);

                        paintItemFlickable.interactive = false;
                    }
                }
            }

            Button {
                id:         effectedModeButton
                iconSource: "qrc:/resources/images/mode_effect_grayscale.png"
                enabled:    false

                onCheckedChanged: {
                    if (checked) {
                        paintItem.setMode(PaintPageScript.MODE_EFFECTED);

                        paintItemFlickable.interactive = false;
                    }
                }
            }

            Button {
                id:         huePickerButton
                iconSource: "qrc:/resources/images/hue_picker.png"
                enabled:    false
                visible:    false

                onCheckedChanged: {
                    if (checked) {
                        paintItem.setMode(PaintPageScript.MODE_EFFECTED);

                        paintItemFlickable.interactive = false;

                        hueZoneRectangle.visible = true;
                    } else {
                        hueZoneRectangle.visible = false;
                    }
                }
            }
        }

        Button {
            id:            scaleButton
            anchors.top:   parent.top
            anchors.right: parent.right
            width:         paintPage.buttonWidth
            checkable:     true
            iconSource:    "qrc:/resources/images/scale.png"
            enabled:       false

            onCheckedChanged: {
                if (checked) {
                    scaleZoneRectangle.visible = true;
                } else {
                    scaleZoneRectangle.visible = false;
                }
            }
        }
    }

    Flickable {
        id:             paintItemFlickable
        anchors.top:    topButtonGroupRectangle.bottom
        anchors.bottom: bottomToolBar.top
        anchors.left:   parent.left
        anchors.right:  parent.right
        contentWidth:   paintItem.width
        contentHeight:  paintItem.height

        PaintItem {
            id: paintItem

            onImageOpened: {
                paintPage.imageWasChanged = true;

                waitRectangle.visible = false;

                scrollModeButton.enabled   = true;
                originalModeButton.enabled = true;
                effectedModeButton.enabled = true;
                huePickerButton.enabled    = true;
                scaleButton.enabled        = true;
                saveToolButton.enabled     = true;
                effectToolButton.enabled   = true;

                scaleSlider.value = PaintPageScript.SCALE_PERCENT_100;
            }

            onImageOpenFailed: {
                waitRectangle.visible = false;

                imageOpenFailedQueryDialog.open();
            }

            onImageSaved: {
                paintPage.imageWasChanged = false;

                if (paintPage.imageOpenRequested) {
                    paintPage.imageOpenRequested = false;

                    mainPageStack.replace(fileOpenPage);
                } else if (paintPage.quitRequested) {
                    Qt.quit();
                }
            }

            onImageSaveFailed: {
                paintPage.imageOpenRequested = false;
                paintPage.quitRequested      = false;

                imageSaveFailedQueryDialog.open();
            }

            onImageChanged: {
                paintPage.imageWasChanged = true;
            }

            onUndoStackEmpty: {
                undoButton.enabled = false;
            }

            onUndoStackNotEmpty: {
                undoButton.enabled = true;
            }
        }
    }

    ToolBar {
        id:             bottomToolBar
        anchors.bottom: parent.bottom
        z:              1

        tools: ToolBarLayout {
            ButtonRow {
                id:        bottomToolBarButtonRow
                exclusive: false

                ToolButton {
                    id:         openToolButton
                    iconSource: "qrc:/resources/images/open.png"
                    flat:       true

                    onClicked: {
                        if (paintPage.imageWasChanged) {
                            paintPage.imageOpenRequested = true;

                            imageSaveQueryDialog.open();
                        } else {
                            mainPageStack.replace(fileOpenPage);
                        }
                    }
                }

                ToolButton {
                    id:         saveToolButton
                    iconSource: "qrc:/resources/images/save.png"
                    flat:       true
                    enabled:    false

                    onClicked: {
                        fileSavePage.setFileUrl(paintItem.currentImageUrl);

                        mainPageStack.replace(fileSavePage);
                    }
                }

                ToolButton {
                    id:         effectToolButton
                    iconSource: "qrc:/resources/images/effect_sketch.png"
                    flat:       true
                    enabled:    false

                    onClicked: {
                        modeButtonRow.checkedButton = scrollModeButton;

                        if (paintPage.currentEffect === PaintPageScript.EFFECT_GRAYSCALE) {
                            paintPage.currentEffect = PaintPageScript.EFFECT_SKETCH;

                            paintItem.setEffect(paintPage.currentEffect);

                            effectedModeButton.iconSource = "qrc:/resources/images/mode_effect_sketch.png";

                            iconSource = "qrc:/resources/images/effect_blur.png";
                        } else if (paintPage.currentEffect === PaintPageScript.EFFECT_SKETCH) {
                            paintPage.currentEffect = PaintPageScript.EFFECT_BLUR;

                            paintItem.setEffect(paintPage.currentEffect);

                            effectedModeButton.iconSource = "qrc:/resources/images/mode_effect_blur.png";

                            iconSource = "qrc:/resources/images/effect_hue.png";
                        } else if (paintPage.currentEffect === PaintPageScript.EFFECT_BLUR) {
                            paintPage.currentEffect = PaintPageScript.EFFECT_HUE;

                            paintItem.setEffect(paintPage.currentEffect);

                            effectedModeButton.iconSource = "qrc:/resources/images/mode_effect_hue.png";
                            huePickerButton.visible       = true;

                            iconSource = "qrc:/resources/images/effect_grayscale.png";
                        } else {
                            paintPage.currentEffect = PaintPageScript.EFFECT_GRAYSCALE;

                            paintItem.setEffect(paintPage.currentEffect);

                            effectedModeButton.iconSource = "qrc:/resources/images/mode_effect_grayscale.png";
                            huePickerButton.visible       = false;

                            iconSource = "qrc:/resources/images/effect_sketch.png";
                        }
                    }
                }

                ToolButton {
                    id:         helpToolButton
                    iconSource: "qrc:/resources/images/help.png"
                    flat:       true

                    onClicked: {
                        mainPageStack.replace(helpPage);
                    }
                }

                ToolButton {
                    id:         exitToolButton
                    iconSource: "qrc:/resources/images/exit.png"
                    flat:       true

                    onClicked: {
                        quitQueryDialog.open();
                    }
                }
            }
        }
    }

    Rectangle {
        id:            scaleZoneRectangle
        anchors.left:  parent.left
        anchors.right: parent.right
        height:        paintPage.scaleZoneHeight
        y:             topButtonGroupRectangle.height
        z:             1
        color:         "black"
        visible:       false

        Text {
            id:                  scalePercentText
            anchors.top:         parent.top
            anchors.bottom:      parent.bottom
            anchors.left:        parent.left
            width:               paintPage.scalePercentTextWidth
            color:               "white"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment:   Text.AlignVCenter
            text:                "100%"
        }

        Slider {
            id:             scaleSlider
            anchors.top:    parent.top
            anchors.bottom: parent.bottom
            anchors.left:   scalePercentText.right
            anchors.right:  parent.right
            orientation:    Qt.Horizontal
            minimumValue:   PaintPageScript.SCALE_PERCENT_50
            maximumValue:   PaintPageScript.SCALE_PERCENT_200
            value:          PaintPageScript.SCALE_PERCENT_100
            stepSize:       1

            onValueChanged: {
                if (value === PaintPageScript.SCALE_PERCENT_50) {
                    scalePercentText.text = "50%";

                    paintItem.setScale(50);
                } else if (value === PaintPageScript.SCALE_PERCENT_75) {
                    scalePercentText.text = "75%";

                    paintItem.setScale(75);
                } else if (value === PaintPageScript.SCALE_PERCENT_100) {
                    scalePercentText.text = "100%";

                    paintItem.setScale(100);
                } else if (value === PaintPageScript.SCALE_PERCENT_150) {
                    scalePercentText.text = "150%";

                    paintItem.setScale(150);
                } else if (value === PaintPageScript.SCALE_PERCENT_200) {
                    scalePercentText.text = "200%";

                    paintItem.setScale(200);
                }
            }
        }
    }

    Rectangle {
        id:                     hueZoneRectangle
        anchors.right:          parent.right
        anchors.verticalCenter: parent.verticalCenter
        width:                  paintPage.hueZoneWidth
        height:                 paintPage.hueZoneHeight
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
            id:           hueZoneRectangleMouseArea
            anchors.fill: parent

            Rectangle {
                id:            hueSliderRectangle
                anchors.left:  parent.left
                anchors.right: parent.right
                height:        paintPage.hueSliderHeight
                y:             (parent.height - height) * 0.5
                z:             2
                color:         "transparent"
                border.color:  "black"
                border.width:  2
            }

            onPositionChanged: {
                hueSliderRectangle.y = Math.max(0, Math.min(height - hueSliderRectangle.height, mouse.y));

                paintItem.setHue((height - Math.max(0, Math.min(height, mouse.y))) * 1.5);
            }

            onPressed: {
                hueSliderRectangle.y = Math.max(0, Math.min(height - hueSliderRectangle.height, mouse.y));

                paintItem.setHue((height - Math.max(0, Math.min(height, mouse.y))) * 1.5);
            }
        }
    }

    Rectangle {
        id:           waitRectangle
        anchors.fill: parent
        z:            10
        color:        "black"
        opacity:      0.75
        visible:      false

        MouseArea {
            id:           waitRectangleMouseArea
            anchors.fill: parent

            Image {
                id:                       waitBusyIndicatorImage
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter:   parent.verticalCenter
                source:                   "qrc:/resources/images/busy_indicator.png"

                NumberAnimation on rotation {
                    running: waitRectangle.visible
                    from:    0
                    to:      360
                    loops:   Animation.Infinite
                }
            }
        }
    }

    QueryDialog {
        id:               openExampleQueryDialog
        titleText:        "Question"
        icon:             "qrc:/resources/images/dialog_question.png"
        message:          "You are running MagicPhotos at the first time. Do you want to open an example image to explore the functionality?"
        acceptButtonText: "Yes"
        rejectButtonText: "No"

        onAccepted: {
            paintItem.openImageFromResource(paintPage.exampleImageResource);

            modeButtonRow.checkedButton = originalModeButton;
        }

        onRejected: {
            waitRectangle.visible = false;
        }
    }

    QueryDialog {
        id:               imageSaveQueryDialog
        titleText:        "Warning"
        icon:             "qrc:/resources/images/dialog_warning.png"
        message:          "Image was changed. Do you want to save it now?"
        acceptButtonText: "Yes"
        rejectButtonText: "No"

        onAccepted: {
            fileSavePage.setFileUrl(paintItem.currentImageUrl);

            mainPageStack.replace(fileSavePage);
        }

        onRejected: {
            if (paintPage.imageOpenRequested) {
                paintPage.imageOpenRequested = false;

                mainPageStack.replace(fileOpenPage);
            } else if (paintPage.quitRequested) {
                Qt.quit();
            }
        }
    }

    QueryDialog {
        id:               imageOpenFailedQueryDialog
        titleText:        "Error"
        icon:             "qrc:/resources/images/dialog_error.png"
        message:          "Could not open image"
        acceptButtonText: "OK"
    }

    QueryDialog {
        id:               imageSaveFailedQueryDialog
        titleText:        "Error"
        icon:             "qrc:/resources/images/dialog_error.png"
        message:          "Could not save image"
        acceptButtonText: "OK"
    }

    QueryDialog {
        id:               quitQueryDialog
        titleText:        "Quit"
        icon:             "qrc:/resources/images/dialog_question.png"
        message:          "Are you sure you want to quit?"
        acceptButtonText: "Yes"
        rejectButtonText: "No"

        onAccepted: {
            if (paintPage.imageWasChanged) {
                paintPage.quitRequested = true;

                imageSaveQueryDialog.open();
            } else {
                Qt.quit();
            }
        }
    }

    Timer {
        id:       offerExampleImageTimer
        interval: 100

        onTriggered: {
            openExampleQueryDialog.open();
        }
    }
}
