import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.3
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2

import "../Misc"

import "../../Util.js" as UtilScript

Dialog {
    id: adMobConsentDialog

    signal showPersonalizedAds()
    signal showNonPersonalizedAds()

    contentItem: Pane {
        implicitWidth:  UtilScript.pt(300)
        implicitHeight: UtilScript.pt(300)

        ColumnLayout {
            anchors.fill:         parent
            anchors.topMargin:    UtilScript.pt(16)
            anchors.bottomMargin: UtilScript.pt(16)
            spacing:              UtilScript.pt(16)

            Label {
                leftPadding:         UtilScript.pt(16)
                rightPadding:        UtilScript.pt(16)
                text:                qsTr("We keep this app free by showing ads. Ad network will <a href=\"https://policies.google.com/technologies/ads\">collect data and use a unique identifier on your device</a> to show you ads. <b>Do you allow to use your data to tailor ads for you?</b>")
                color:               "black"
                font.pointSize:      16
                font.family:         "Helvetica"
                horizontalAlignment: Text.AlignJustify
                verticalAlignment:   Text.AlignVCenter
                wrapMode:            Text.Wrap
                fontSizeMode:        Text.Fit
                minimumPointSize:    8
                textFormat:          Text.StyledText
                Layout.fillWidth:    true
                Layout.fillHeight:   true

                onLinkActivated: {
                    Qt.openUrlExternally(link);
                }
            }

            MFButton {
                implicitWidth:    UtilScript.pt(280)
                implicitHeight:   UtilScript.pt(64)
                text:             qsTr("Yes, show me relevant ads")
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                onClicked: {
                    adMobConsentDialog.showPersonalizedAds();
                    adMobConsentDialog.close();
                }
            }

            MFButton {
                implicitWidth:    UtilScript.pt(280)
                implicitHeight:   UtilScript.pt(64)
                text:             qsTr("No, show me ads that are less relevant")
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                onClicked: {
                    adMobConsentDialog.showNonPersonalizedAds();
                    adMobConsentDialog.close();
                }
            }
        }
    }
}
