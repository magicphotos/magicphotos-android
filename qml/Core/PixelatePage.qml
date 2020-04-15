import QtQuick 2.12
import ImageEditor 1.0

import "Templates"

EffectEditorPage {
    id: pixelatePage

    editorComponent: Component {
        PixelateEditor {
        }
    }

    property int imageOrientation:    -1
    property int pixDenom:            -1

    property string imagePath:        ""

    Component.onCompleted: {
        if (imageOrientation !== -1 && pixDenom !== -1 && imagePath !== "") {
            editor.pixDenom = pixDenom;

            editor.openImage(imagePath, imageOrientation);
        }
    }
}
