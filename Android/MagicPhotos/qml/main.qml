import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtPurchasing 1.0

import "Core"

import "Util.js" as UtilScript

ApplicationWindow {
    id:      mainWindow
    visible: true

    Material.theme:   Material.System
    Material.primary: Material.Teal

    property bool   fullVersion:          false

    property string interstitialAdUnitId: "ca-app-pub-3940256099942544/1033173712"
    property string adViewUnitId:         "ca-app-pub-3940256099942544/6300978111"
    property string bannerSize:           "FLUID"
    property string testDeviceId:         ""

    function purchaseFullVersion() {
        fullVersionProduct.purchase();
    }

    function adViewHeightUpdated(banner_height) {
        if (mainStackView.depth > 0 && mainStackView.currentItem.hasOwnProperty("bannerHeight")) {
            mainStackView.currentItem.bannerHeight = banner_height;
        }
    }

    Component.onCompleted: {
        AppSettings.defaultBrushSize = UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 16);

        fullVersion = AppSettings.isFullVersion;

        AndroidGW.adViewHeightUpdated.connect(adViewHeightUpdated);
        AndroidGW.prepareInterstitialAd(interstitialAdUnitId, testDeviceId);

        mainStackView.push(modeSelectionPage);
    }

    onFullVersionChanged: {
        AppSettings.isFullVersion = fullVersion;

        if (mainStackView.depth > 0 && mainStackView.currentItem.hasOwnProperty("bannerHeight")) {
            if (fullVersion) {
                mainStackView.currentItem.bannerHeight = 0;

                AndroidGW.hideAdView();
            } else {
                AndroidGW.showAdView(mainWindow.adViewUnitId, mainWindow.bannerSize, mainWindow.testDeviceId);
            }
        }
    }

    Store {
        Product {
            id:         fullVersionProduct
            identifier: "magicphotos.version.full"
            type:       Product.Unlockable

            onPurchaseSucceeded: {
                mainWindow.fullVersion = true;

                transaction.finalize();
            }

            onPurchaseRestored: {
                mainWindow.fullVersion = true;

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
                get(depth - 1).forceActiveFocus();

                if (item.hasOwnProperty("bannerHeight")) {
                    if (mainWindow.fullVersion) {
                        item.bannerHeight = 0;

                        AndroidGW.hideAdView();
                    } else {
                        AndroidGW.showAdView(mainWindow.adViewUnitId, mainWindow.bannerSize, mainWindow.testDeviceId);
                    }
                } else {
                    AndroidGW.hideAdView();
                }

                if (item.hasOwnProperty("allowInterstitial") && item.allowInterstitial && !mainWindow.fullVersion) {
                    AndroidGW.showInterstitialAd();
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
}
