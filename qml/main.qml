import QtQuick 2.9
import QtQuick.Window 2.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtPurchasing 1.0

import "Core"

import "Util.js" as UtilScript

ApplicationWindow {
    id:                           mainWindow
    visible:                      true
    Screen.orientationUpdateMask: Qt.PortraitOrientation         | Qt.LandscapeOrientation |
                                  Qt.InvertedPortraitOrientation | Qt.InvertedLandscapeOrientation
    Material.theme:               Material.System
    Material.primary:             Material.Teal

    property bool disableAds:       false
    property int screenOrientation: Screen.orientation

    onDisableAdsChanged: {
        AppSettings.disableAds = disableAds;

        updateFeatures();
    }

    onScreenOrientationChanged: {
        if (mainStackView.depth > 0 && mainStackView.currentItem.hasOwnProperty("bannerViewHeight")) {
            if (disableAds) {
                AdMobHelper.hideBannerView();
            } else {
                AdMobHelper.showBannerView();
            }
        }
    }

    function updateFeatures() {
        if (mainStackView.depth > 0 && mainStackView.currentItem.hasOwnProperty("bannerViewHeight")) {
            if (disableAds) {
                AdMobHelper.hideBannerView();
            } else {
                AdMobHelper.showBannerView();
            }
        }
    }

    Store {
        Product {
            id:         disabledAdsProduct
            identifier: "vkgeo.unlockable.disabledads"
            type:       Product.Unlockable

            onPurchaseSucceeded: {
                mainWindow.disableAds = true;

                transaction.finalize();
            }

            onPurchaseRestored: {
                mainWindow.disableAds = true;

                transaction.finalize();
            }

            onPurchaseFailed: {
                if (transaction.failureReason === Transaction.ErrorOccurred) {
                    console.log(transaction.errorString);
                }

                transaction.finalize();
            }
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
                    if (disableAds) {
                        AdMobHelper.hideBannerView();
                    } else {
                        AdMobHelper.showBannerView();
                    }
                } else {
                    AdMobHelper.hideBannerView();
                }

                if (currentItem.hasOwnProperty("allowInterstitial") && currentItem.allowInterstitial && !mainWindow.disableAds) {
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

        disableAds = AppSettings.disableAds;

        updateFeatures();

        AdMobHelper.initialize();

        mainStackView.push(modeSelectionPage);
    }
}
