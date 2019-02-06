import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.3

import "Core"

import "Util.js" as UtilScript

ApplicationWindow {
    id:                           mainWindow
    title:                        qsTr("MagicPhotos")
    visible:                      true
    Screen.orientationUpdateMask: Qt.PortraitOrientation         | Qt.LandscapeOrientation |
                                  Qt.InvertedPortraitOrientation | Qt.InvertedLandscapeOrientation
    Material.theme:               Material.System
    Material.primary:             Material.Teal

    property int screenOrientation: Screen.orientation

    onScreenOrientationChanged: {
        if (mainStackView.depth > 0 && mainStackView.currentItem.hasOwnProperty("bannerViewHeight")) {
            AdMobHelper.showBannerView();
        }
    }

    StackView {
        id:           mainStackView
        anchors.fill: parent

        onCurrentItemChanged: {
            for (var i = 0; i < depth; i++) {
                var item = get(i, false);

                if (item !== null) {
                    item.focus = false;
                }
            }

            if (depth > 0) {
                currentItem.forceActiveFocus();

                if (currentItem.hasOwnProperty("bannerViewHeight")) {
                    AdMobHelper.showBannerView();
                } else {
                    AdMobHelper.hideBannerView();
                }

                if (currentItem.hasOwnProperty("allowInterstitial") && currentItem.allowInterstitial) {
                    AdMobHelper.showInterstitial();
                }
            }
        }
    }

    ModeSelectionPage {
        id: modeSelectionPage
    }

    MouseArea {
        anchors.fill: parent
        z:            20
        enabled:      mainStackView.busy
    }

    Component.onCompleted: {
        AppSettings.defaultBrushSize = UtilScript.pt(16);

        mainStackView.push(modeSelectionPage);
    }
}
