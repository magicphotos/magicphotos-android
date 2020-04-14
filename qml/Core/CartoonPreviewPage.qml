import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.3
import QtQuick.Layouts 1.12
import ImageEditor 1.0

import "Templates"

import "../Util.js" as UtilScript

PreviewPage {
    id:              cartoonPreviewPage
    headerLabelText: qsTr("Cartoon")

    previewGeneratorComponent: Component {
        CartoonPreviewGenerator {
            property int waitRectangleUsageCounter: 0
        }
    }

    controlColumnLayoutComponent: Component {
        ColumnLayout {
            spacing: UtilScript.dp(UIHelper.screenDpi, 16)

            Slider {
                from:             cartoonPreviewPage.gaussianRadiusSliderFrom
                to:               cartoonPreviewPage.gaussianRadiusSliderTo
                value:            cartoonPreviewPage.gaussianRadiusSliderValue
                stepSize:         cartoonPreviewPage.gaussianRadiusSliderStepSize
                Layout.fillWidth: true

                onPressedChanged: {
                    if (!pressed) {
                        cartoonPreviewPage.previewGenerator.radius = value;
                    }
                }
            }

            Slider {
                from:             cartoonPreviewPage.thresholdSliderFrom
                to:               cartoonPreviewPage.thresholdSliderTo
                value:            cartoonPreviewPage.thresholdSliderValue
                stepSize:         cartoonPreviewPage.thresholdSliderStepSize
                Layout.fillWidth: true

                onPressedChanged: {
                    if (!pressed) {
                        cartoonPreviewPage.previewGenerator.threshold = value;
                    }
                }
            }
        }
    }

    readonly property real gaussianRadiusSliderFrom:     0.0
    readonly property real gaussianRadiusSliderTo:       10.0
    readonly property real gaussianRadiusSliderValue:    5.0
    readonly property real gaussianRadiusSliderStepSize: 1.0
    readonly property real thresholdSliderFrom:          32.0
    readonly property real thresholdSliderTo:            128.0
    readonly property real thresholdSliderValue:         80.0
    readonly property real thresholdSliderStepSize:      8.0

    property bool componentCompleted:                    false

    property int imageOrientation:                       -1

    property string imagePath:                           ""

    onComponentCompletedChanged: {
        if (componentCompleted && imageOrientation !== -1 && imagePath !== "") {
            previewGenerator.radius    = gaussianRadiusSliderValue;
            previewGenerator.threshold = thresholdSliderValue;

            previewGenerator.openImage(imagePath, imageOrientation);
        }
    }

    onImageOrientationChanged: {
        if (componentCompleted && imageOrientation !== -1 && imagePath !== "") {
            previewGenerator.radius    = gaussianRadiusSliderValue;
            previewGenerator.threshold = thresholdSliderValue;

            previewGenerator.openImage(imagePath, imageOrientation);
        }
    }

    onImagePathChanged: {
        if (componentCompleted && imageOrientation !== -1 && imagePath !== "") {
            previewGenerator.radius    = gaussianRadiusSliderValue;
            previewGenerator.threshold = thresholdSliderValue;

            previewGenerator.openImage(imagePath, imageOrientation);
        }
    }

    onApplyClicked: {
        var control_values = getControlValues();

        if (control_values.length === 2) {
            var component = Qt.createComponent("CartoonPage.qml");

            if (component.status === Component.Ready) {
                mainStackView.push(component, {"imageOrientation": imageOrientation, "gaussianRadius": control_values[0], "cartoonThreshold": control_values[1], "imagePath": imagePath});
            } else {
                console.error(component.errorString());
            }
        }
    }

    Component.onCompleted: {
        componentCompleted = true;
    }
}
