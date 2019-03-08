import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.3
import QtPurchasing 1.0

import "Util.js" as UtilScript

ApplicationWindow {
    id:                           mainWindow
    title:                        qsTr("MagicPhotos")
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
        } else {
            AdMobHelper.hideBannerView();
        }
    }

    function updateFeatures() {
        if (mainStackView.depth > 0 && mainStackView.currentItem.hasOwnProperty("bannerViewHeight")) {
            if (disableAds) {
                AdMobHelper.hideBannerView();
            } else {
                AdMobHelper.showBannerView();
            }
        } else {
            AdMobHelper.hideBannerView();
        }
    }

    Store {
        Product {
            id:         disabledAdsProduct
            identifier: "magicphotos.unlockable.disabledads"
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
                var item = get(i, StackView.DontLoad);

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
            } else {
                AdMobHelper.hideBannerView();
            }
        }
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

        var component = Qt.createComponent("Core/ModeSelectionPage.qml");

        if (component.status === Component.Ready) {
            mainStackView.push(component);
        } else {
            console.log(component.errorString());
        }
    }
}
