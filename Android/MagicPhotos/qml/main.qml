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

    property bool fullVersion:      false

    property int screenOrientation: Screen.orientation

    onFullVersionChanged: {
        AppSettings.isFullVersion = fullVersion;

        if (mainStackView.depth > 0 && mainStackView.currentItem.hasOwnProperty("adViewHeight")) {
            if (fullVersion) {
                AndroidGW.hideAdView();
            } else {
                AndroidGW.showAdView();
            }
        }
    }

    onScreenOrientationChanged: {
        if (mainStackView.depth > 0 && mainStackView.currentItem.hasOwnProperty("adViewHeight")) {
            if (fullVersion) {
                AndroidGW.hideAdView();
            } else {
                AndroidGW.showAdView();
            }
        }
    }

    function purchaseFullVersion() {
        fullVersionProduct.purchase();
    }

    function adViewHeightUpdated(adview_height) {
        if (mainStackView.depth > 0 && mainStackView.currentItem.hasOwnProperty("adViewHeight")) {
            mainStackView.currentItem.adViewHeight = adview_height;
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
                currentItem.forceActiveFocus();

                if (currentItem.hasOwnProperty("adViewHeight")) {
                    if (mainWindow.fullVersion) {
                        AndroidGW.hideAdView();
                    } else {
                        AndroidGW.showAdView();
                    }
                } else {
                    AndroidGW.hideAdView();
                }

                if (currentItem.hasOwnProperty("allowInterstitialAd") && currentItem.allowInterstitialAd && !mainWindow.fullVersion) {
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

    Component.onCompleted: {
        AppSettings.defaultBrushSize = UtilScript.mapSizeToDevice(AndroidGW.getScreenDPI(), 16);

        fullVersion = AppSettings.isFullVersion;

        AndroidGW.adViewHeightUpdated.connect(adViewHeightUpdated);
        AndroidGW.createInterstitialAd();

        mainStackView.push(modeSelectionPage);
    }
}
