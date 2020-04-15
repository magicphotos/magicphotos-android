import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.3
import QtQuick.Layouts 1.12
import ImageEditor 1.0

import "Templates"

import "../Util.js" as UtilScript

PreviewPage {
    id:              sketchPreviewPage
    headerLabelText: qsTr("Sketch")

    previewGeneratorComponent: Component {
        SketchPreviewGenerator {
            property int waitRectangleUsageCounter: 0
        }
    }

    controlColumnLayoutComponent: Component {
        ColumnLayout {
            spacing: UtilScript.dp(UIHelper.screenDpi, 16)

            Slider {
                from:             sketchPreviewPage.gaussianRadiusSliderFrom
                to:               sketchPreviewPage.gaussianRadiusSliderTo
                value:            sketchPreviewPage.gaussianRadiusSliderValue
                stepSize:         sketchPreviewPage.gaussianRadiusSliderStepSize
                Layout.fillWidth: true

                onPressedChanged: {
                    if (!pressed) {
                        sketchPreviewPage.previewGenerator.radius = value;
                    }
                }
            }
        }
    }

    readonly property real gaussianRadiusSliderFrom:     4.0
    readonly property real gaussianRadiusSliderTo:       18.0
    readonly property real gaussianRadiusSliderValue:    11.0
    readonly property real gaussianRadiusSliderStepSize: 1.0

    property int imageOrientation:                       -1

    property string imagePath:                           ""

    onApplyClicked: {
        var control_values = getControlValues();

        if (control_values.length === 1) {
            var component = Qt.createComponent("SketchPage.qml");

            if (component.status === Component.Ready) {
                mainStackView.push(component, {"imageOrientation": imageOrientation, "gaussianRadius": control_values[0], "imagePath": imagePath});
            } else {
                console.error(component.errorString());
            }
        }
    }

    Component.onCompleted: {
        if (imageOrientation !== -1 && imagePath !== "") {
            previewGenerator.radius = gaussianRadiusSliderValue;

            previewGenerator.openImage(imagePath, imageOrientation);
        }
    }
}
