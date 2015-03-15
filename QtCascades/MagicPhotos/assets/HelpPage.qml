import bb.cascades 1.3
import bb.system 1.2
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
            title:               qsTr("Review App")
            imageSource:         "images/review.png"
            ActionBar.placement: ActionBarPlacement.Signature

            onTriggered: {
                if (TrialManager.trialMode) {
                    appWorldTrialInvocation.trigger("bb.action.OPEN");
                } else {
                    appWorldFullInvocation.trigger("bb.action.OPEN");
                }
            }

            attachedObjects: [
                Invocation {
                    id: appWorldTrialInvocation

                    query: InvokeQuery {
                        mimeType: "application/x-bb-appworld"
                        uri:      "appworld://content/30425892"
                    }
                },
                Invocation {
                    id: appWorldFullInvocation

                    query: InvokeQuery {
                        mimeType: "application/x-bb-appworld"
                        uri:      "appworld://content/20356189"
                    }
                }
            ]
        },
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
        accessibility.name: qsTr("Help browser")

        scrollViewProperties {
            scrollMode:         ScrollMode.Both
            pinchToZoomEnabled: true
            minContentScale:    1.0
            maxContentScale:    4.0
        }            

        WebView {
            url:                qsTr("local:///assets/doc/help.html")
            accessibility.name: qsTr("Help browser")
        }
    }
}
