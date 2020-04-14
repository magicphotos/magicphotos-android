import QtQuick 2.12
import ImageEditor 1.0

import "Templates"

EffectEditorPage {
    id: blurPage

    editorComponent: Component {
        BlurEditor {
        }
    }

    property bool componentCompleted: false

    property int imageOrientation:    -1
    property int gaussianRadius:      -1

    property string imagePath:        ""

    onComponentCompletedChanged: {
        if (componentCompleted && imageOrientation !== -1 && gaussianRadius !== -1 && imagePath !== "") {
            editor.radius = gaussianRadius;

            editor.openImage(imagePath, imageOrientation);
        }
    }

    onImageOrientationChanged: {
        if (componentCompleted && imageOrientation !== -1 && gaussianRadius !== -1 && imagePath !== "") {
            editor.radius = gaussianRadius;

            editor.openImage(imagePath, imageOrientation);
        }
    }

    onGaussianRadiusChanged: {
        if (componentCompleted && imageOrientation !== -1 && gaussianRadius !== -1 && imagePath !== "") {
            editor.radius = gaussianRadius;

            editor.openImage(imagePath, imageOrientation);
        }
    }

    onImagePathChanged: {
        if (componentCompleted && imageOrientation !== -1 && gaussianRadius !== -1 && imagePath !== "") {
            editor.radius = gaussianRadius;

            editor.openImage(imagePath, imageOrientation);
        }
    }

    Component.onCompleted: {
        componentCompleted = true;
    }
}
