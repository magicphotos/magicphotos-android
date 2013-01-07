import bb.cascades 1.0

Page {
    id: helpPage
    
    paneProperties: NavigationPaneProperties {
        backButton: ActionItem {
            onTriggered: {
                navigationPane.pop();
            }
        }
    }

    Container {
        background: Color.Black
        
        layout: StackLayout {
        }

        ScrollView {
            WebView {
                id:  helpWebView
                url: qsTr("doc/help.html")
            }
        }
    }
}
