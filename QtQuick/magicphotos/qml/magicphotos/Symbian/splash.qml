import QtQuick 1.0
import com.nokia.symbian 1.0

Window {
    id: splashWindow

    Rectangle {
        id:           backgroundRectangle
        anchors.fill: parent
        color:        "black"

        Image {
            id:                       logoImage
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter:   parent.verticalCenter
            source:                   "qrc:/resources/images/splash.png"
            opacity:                  0.5
        }

        Text {
            id:                       splashText
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top:              logoImage.bottom
            color:                    "white"
            horizontalAlignment:      Text.AlignHCenter
            verticalAlignment:        Text.AlignVCenter
            text:                     "Loading, one moment please..."
        }
    }
}
