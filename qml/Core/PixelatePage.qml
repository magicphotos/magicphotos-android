import QtQuick 2.12
import ImageEditor 1.0

import "Templates"

EffectEditorPage {
    id: pixelatePage

    editorComponent: Component {
        PixelateEditor {
        }
    }

    property bool componentCompleted: false

    property int imageOrientation:    -1
    property int pixDenom:            -1

    property string imagePath:        ""

    onComponentCompletedChanged: {
        if (componentCompleted && imageOrientation !== -1 && pixDenom !== -1 && imagePath !== "") {
            editor.pixDenom = pixDenom;

            editor.openImage(imagePath, imageOrientation);
        }
    }

    onImageOrientationChanged: {
        if (componentCompleted && imageOrientation !== -1 && pixDenom !== -1 && imagePath !== "") {
            editor.pixDenom = pixDenom;

            editor.openImage(imagePath, imageOrientation);
        }
    }

    onPixDenomChanged: {
        if (componentCompleted && imageOrientation !== -1 && pixDenom !== -1 && imagePath !== "") {
            editor.pixDenom = pixDenom;

            editor.openImage(imagePath, imageOrientation);
        }
    }

    onImagePathChanged: {
        if (componentCompleted && imageOrientation !== -1 && pixDenom !== -1 && imagePath !== "") {
            editor.pixDenom = pixDenom;

            editor.openImage(imagePath, imageOrientation);
        }
    }

    Component.onCompleted: {
        componentCompleted = true;
    }
}
