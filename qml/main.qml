import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.3
import QtPurchasing 1.0

import "Core/Dialog"

import "Util.js" as UtilScript

ApplicationWindow {
    id:                           mainWindow
    title:                        qsTr("MagicPhotos")
    visible:                      true
    Screen.orientationUpdateMask: Qt.PortraitOrientation         | Qt.LandscapeOrientation |
                                  Qt.InvertedPortraitOrientation | Qt.InvertedLandscapeOrientation
    Material.theme:               UIHelper.darkTheme ? Material.Dark : Material.Light
    Material.primary:             Material.Teal

    readonly property int screenOrientation: Screen.orientation

    property bool disableAds:                false

    property string adMobConsent:            ""

    onScreenOrientationChanged: {
        if (mainStackView.depth > 0 && typeof mainStackView.currentItem.bannerViewHeight === "number") {
            if (disableAds) {
                AdMobHelper.hideBannerView();
            } else {
                AdMobHelper.showBannerView();
            }
        } else {
            AdMobHelper.hideBannerView();
        }
    }

    onDisableAdsChanged: {
        AppSettings.disableAds = disableAds;

        updateFeatures();
    }

    onAdMobConsentChanged: {
        AppSettings.adMobConsent = adMobConsent;

        updateFeatures();
    }

    function updateFeatures() {
        if (!disableAds && (adMobConsent === "PERSONALIZED" || adMobConsent === "NON_PERSONALIZED")) {
            AdMobHelper.setPersonalization(adMobConsent === "PERSONALIZED");

            AdMobHelper.initAds();
        }

        if (mainStackView.depth > 0 && typeof mainStackView.currentItem.bannerViewHeight === "number") {
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

                if (typeof currentItem.bannerViewHeight === "number") {
                    if (mainWindow.disableAds) {
                        AdMobHelper.hideBannerView();
                    } else {
                        AdMobHelper.showBannerView();
                    }
                } else {
                    AdMobHelper.hideBannerView();
                }

                if (currentItem.allowInterstitial && !mainWindow.disableAds) {
                    AdMobHelper.showInterstitial();
                }
            } else {
                AdMobHelper.hideBannerView();
            }
        }
    }

    MultiPointTouchArea {
        anchors.fill: parent
        z:            1
        enabled:      mainStackView.busy
    }

    AdMobConsentDialog {
        id: adMobConsentDialog

        onPersonalizedAdsSelected: {
            mainWindow.adMobConsent = "PERSONALIZED";
        }

        onNonPersonalizedAdsSelected: {
            mainWindow.adMobConsent = "NON_PERSONALIZED";
        }
    }

    Component.onCompleted: {
        AppSettings.defaultBrushSize = UtilScript.pt(16);

        disableAds   = AppSettings.disableAds;
        adMobConsent = AppSettings.adMobConsent;

        updateFeatures();

        var component = Qt.createComponent("Core/ModeSelectionPage.qml");

        if (component.status === Component.Ready) {
            mainStackView.push(component);
        } else {
            console.log(component.errorString());
        }

        if (!disableAds && adMobConsent !== "PERSONALIZED" && adMobConsent !== "NON_PERSONALIZED") {
            adMobConsentDialog.open();
        }
    }
}
