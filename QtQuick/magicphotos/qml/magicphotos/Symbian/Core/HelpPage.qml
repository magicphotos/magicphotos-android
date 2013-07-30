import QtQuick 1.1
import QtWebKit 1.0
import com.nokia.symbian 1.0

Page {
    id:           helpPage
    anchors.fill: parent

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
            url:             "../../doc/help.html"
        }
    }

    ToolBar {
        id:             bottomToolBar
        anchors.bottom: parent.bottom
        z:              1

        tools: ToolBarLayout {
            ToolButton {
                iconSource: "../images/back.png"
                flat:       true

                onClicked: {
                    mainPageStack.pop();
                }
            }
        }
    }
}
