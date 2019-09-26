import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.3
import QtQuick.Layouts 1.12

import "../Misc"

import "../../Util.js" as UtilScript

Popup {
    id:               adMobConsentDialog
    anchors.centerIn: Overlay.overlay
    padding:          UtilScript.dp(UIHelper.screenDpi, 8)
    modal:            true
    closePolicy:      Popup.NoAutoClose

    signal personalizedAdsSelected()
    signal nonPersonalizedAdsSelected()

    background: Pane {
    }

    contentItem: Rectangle {
        implicitWidth:  UtilScript.dp(UIHelper.screenDpi, 300)
        implicitHeight: UtilScript.dp(UIHelper.screenDpi, 300)
        color:          "transparent"

        ColumnLayout {
            anchors.fill: parent
            spacing:      UtilScript.dp(UIHelper.screenDpi, 8)

            Label {
                text:                qsTr("We keep this app free by showing ads. Ad network will <a href=\"https://policies.google.com/technologies/ads\">collect data and use a unique identifier on your device</a> to show you ads. <b>Do you allow to use your data to tailor ads for you?</b>")
                font.pixelSize:      UtilScript.dp(UIHelper.screenDpi, 16)
                font.family:         "Helvetica"
                horizontalAlignment: Text.AlignJustify
                verticalAlignment:   Text.AlignVCenter
                wrapMode:            Text.Wrap
                fontSizeMode:        Text.Fit
                minimumPixelSize:    UtilScript.dp(UIHelper.screenDpi, 8)
                textFormat:          Text.StyledText
                Layout.fillWidth:    true
                Layout.fillHeight:   true

                onLinkActivated: {
                    Qt.openUrlExternally(link);
                }
            }

            MFButton {
                implicitWidth:    UtilScript.dp(UIHelper.screenDpi, 280)
                implicitHeight:   UtilScript.dp(UIHelper.screenDpi, 64)
                text:             qsTr("Yes, show me relevant ads")
                font.family:      "Helvetica"
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                onClicked: {
                    adMobConsentDialog.personalizedAdsSelected();
                    adMobConsentDialog.close();
                }
            }

            MFButton {
                implicitWidth:    UtilScript.dp(UIHelper.screenDpi, 280)
                implicitHeight:   UtilScript.dp(UIHelper.screenDpi, 64)
                text:             qsTr("No, show me ads that are less relevant")
                font.family:      "Helvetica"
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                onClicked: {
                    adMobConsentDialog.nonPersonalizedAdsSelected();
                    adMobConsentDialog.close();
                }
            }
        }
    }
}
