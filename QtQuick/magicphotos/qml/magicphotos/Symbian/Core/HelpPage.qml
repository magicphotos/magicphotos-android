import QtQuick 1.0
import QtWebKit 1.0
import com.nokia.symbian 1.0

Page {
    id:           helpPage
    anchors.fill: parent

    onStatusChanged: {
        if (status === PageStatus.Active) {
            helpWebView.url = "qrc:/resources/doc/help.html";
        }
    }

    Flickable {
        id:             helpWebViewFlickable
        anchors.top:    parent.top
        anchors.bottom: bottomToolBar.top
        anchors.left:   parent.left
        anchors.right:  parent.right
        contentWidth:   helpWebView.width
        contentHeight:  helpWebView.height

        WebView {
            id:              helpWebView
            preferredWidth:  helpWebViewFlickable.width
            preferredHeight: helpWebViewFlickable.height
        }
    }

    ToolBar {
        id:             bottomToolBar
        anchors.bottom: parent.bottom
        z:              1

        tools: ToolBarLayout {
            ToolButton {
                id:         helpOkToolButton
                iconSource: "qrc:/resources/images/ok.png"
                flat:       true

                onClicked: {
                    mainPageStack.replace(paintPage);
                }
            }
        }
    }
}
