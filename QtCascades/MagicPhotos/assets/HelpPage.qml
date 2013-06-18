import bb.cascades 1.0
import bb.system 1.0
import DownloadInviter 1.0

Page {
    id: helpPage
    
    paneProperties: NavigationPaneProperties {
        backButton: ActionItem {
            onTriggered: {
                navigationPane.pop();
            }
        }
    }

    actions: [
        ActionItem {
            title:               qsTr("Recommend App")
            imageSource:         "images/share.png"
            ActionBar.placement: ActionBarPlacement.OnBar

            onTriggered: {
                downloadInviter.sendDownloadInvitation();
            }
            
            attachedObjects: [
                DownloadInviter {
                    id: downloadInviter

                    onInvitationSendFailed: {
                        invitationSendFailedToast.show();
                    }
                },
                SystemToast {
                    id:   invitationSendFailedToast
                    body: qsTr("Could not send download invitation")
                }
            ]
        }
    ]

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
