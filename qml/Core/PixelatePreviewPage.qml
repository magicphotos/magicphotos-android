import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.3
import QtQuick.Layouts 1.12
import ImageEditor 1.0

import "Templates"

import "../Util.js" as UtilScript

PreviewPage {
    id:              pixelatePreviewPage
    headerLabelText: qsTr("Pixelate")

    previewGeneratorComponent: Component {
        PixelatePreviewGenerator {
            property int waitRectangleUsageCounter: 0
        }
    }

    controlColumnLayoutComponent: Component {
        ColumnLayout {
            spacing: UtilScript.dp(UIHelper.screenDpi, 16)

            Slider {
                from:             pixelatePreviewPage.pixDenomSliderFrom
                to:               pixelatePreviewPage.pixDenomSliderTo
                value:            pixelatePreviewPage.pixDenomSliderValue
                stepSize:         pixelatePreviewPage.pixDenomSliderStepSize
                Layout.fillWidth: true

                onPressedChanged: {
                    if (!pressed) {
                        pixelatePreviewPage.previewGenerator.pixDenom = value;
                    }
                }
            }
        }
    }

    readonly property real pixDenomSliderFrom:     32.0
    readonly property real pixDenomSliderTo:       192.0
    readonly property real pixDenomSliderValue:    112.0
    readonly property real pixDenomSliderStepSize: 8.0

    property int imageOrientation:                 -1

    property string imagePath:                     ""

    onApplyClicked: {
        var control_values = getControlValues();

        if (control_values.length === 1) {
            var component = Qt.createComponent("PixelatePage.qml");

            if (component.status === Component.Ready) {
                mainStackView.push(component, {"imageOrientation": imageOrientation, "pixDenom": control_values[0], "imagePath": imagePath});
            } else {
                console.error(component.errorString());
            }
        }
    }

    Component.onCompleted: {
        if (imageOrientation !== -1 && imagePath !== "") {
            previewGenerator.pixDenom = pixDenomSliderValue;

            previewGenerator.openImage(imagePath, imageOrientation);
        }
    }
}
