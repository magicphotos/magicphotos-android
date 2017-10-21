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

    property bool   fullVersion:  false

    property string adUnitId:     "ca-app-pub-3940256099942544/6300978111"
    property string bannerSize:   "BANNER"
    property string testDeviceId: ""

    function purchaseFullVersion() {
        fullVersionProduct.purchase();
    }

    Component.onCompleted: {
        AppSettings.defaultBrushSize = UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 16);

        fullVersion = AppSettings.isFullVersion;

        mainStackView.push(modeSelectionPage);
    }

    onFullVersionChanged: {
        AppSettings.isFullVersion = fullVersion;

        if (fullVersion && mainStackView.depth > 0 && mainStackView.currentItem.hasOwnProperty("bannerHeight")) {
            mainStackView.currentItem.bannerHeight = 0;

            AndroidGW.hideAdView();
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
                        if (mainWindow.bannerSize === "BANNER") {
                            item.bannerHeight = UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 50);
                        } else if (mainWindow.bannerSize === "FULL_BANNER") {
                            item.bannerHeight = UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 60);
                        } else if (mainWindow.bannerSize === "LARGE_BANNER") {
                            item.bannerHeight = UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 100);
                        } else if (mainWindow.bannerSize === "LEADERBOARD") {
                            item.bannerHeight = UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 90);
                        } else if (mainWindow.bannerSize === "MEDIUM_RECTANGLE") {
                            item.bannerHeight = UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 250);
                        } else {
                            item.bannerHeight = 0;
                        }

                        AndroidGW.showAdView(mainWindow.adUnitId, mainWindow.bannerSize, mainWindow.testDeviceId);
                    }
                } else {
                    AndroidGW.hideAdView();
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
