import QtQuick 1.0
import com.nokia.symbian 1.1

Window {
    id: splashWindow

    Rectangle {
        anchors.fill: parent
        color:        "black"

        Image {
            id:               logoImage
            anchors.centerIn: parent
            source:           "../images/splash.png"
            opacity:          0.5
        }

        Text {
            anchors.top:              logoImage.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            color:                    "white"
            horizontalAlignment:      Text.AlignHCenter
            verticalAlignment:        Text.AlignVCenter
            text:                     "Loading, one moment please..."
        }
    }
}
