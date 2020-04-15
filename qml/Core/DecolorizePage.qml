import QtQuick 2.12
import ImageEditor 1.0

import "Templates"

EffectEditorPage {
    id: decolorizePage

    editorComponent: Component {
        DecolorizeEditor {
        }
    }

    property int imageOrientation:    -1

    property string imagePath:        ""

    Component.onCompleted: {
        if (imageOrientation !== -1 && imagePath !== "") {
            editor.openImage(imagePath, imageOrientation);
        }
    }
}
