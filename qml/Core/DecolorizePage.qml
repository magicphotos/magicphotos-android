import QtQuick 2.12
import ImageEditor 1.0

import "Templates"

EffectEditorPage {
    id: decolorizePage

    editorComponent: Component {
        DecolorizeEditor {
        }
    }

    property bool componentCompleted: false

    property int imageOrientation:    -1

    property string imagePath:        ""

    onComponentCompletedChanged: {
        if (componentCompleted && imageOrientation !== -1 && imagePath !== "") {
            editor.openImage(imagePath, imageOrientation);
        }
    }

    onImageOrientationChanged: {
        if (componentCompleted && imageOrientation !== -1 && imagePath !== "") {
            editor.openImage(imagePath, imageOrientation);
        }
    }

    onImagePathChanged: {
        if (componentCompleted && imageOrientation !== -1 && imagePath !== "") {
            editor.openImage(imagePath, imageOrientation);
        }
    }

    Component.onCompleted: {
        componentCompleted = true;
    }
}
