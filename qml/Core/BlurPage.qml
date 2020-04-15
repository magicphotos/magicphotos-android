import QtQuick 2.12
import ImageEditor 1.0

import "Templates"

EffectEditorPage {
    id: blurPage

    editorComponent: Component {
        BlurEditor {
        }
    }

    property int imageOrientation:    -1
    property int gaussianRadius:      -1

    property string imagePath:        ""

    Component.onCompleted: {
        if (imageOrientation !== -1 && gaussianRadius !== -1 && imagePath !== "") {
            editor.radius = gaussianRadius;

            editor.openImage(imagePath, imageOrientation);
        }
    }
}
