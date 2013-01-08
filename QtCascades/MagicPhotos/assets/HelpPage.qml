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

    ScrollView {
        scrollViewProperties {
            scrollMode:         ScrollMode.Both
            pinchToZoomEnabled: true
            minContentScale:    1.0
            maxContentScale:    4.0
        }            

        WebView {
            url: qsTr("doc/help.html")
        }
    }
}
