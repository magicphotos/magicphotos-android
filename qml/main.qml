import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.3

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

    readonly property int screenDpi:         UIHelper.screenDpi
    readonly property int screenOrientation: Screen.orientation

    property string adMobConsent:            ""

    onScreenDpiChanged: {
        if (mainStackView.depth > 0 && typeof mainStackView.currentItem.bannerViewHeight === "number") {
            AdMobHelper.showBannerView();
        } else {
            AdMobHelper.hideBannerView();
        }
    }

    onScreenOrientationChanged: {
        if (mainStackView.depth > 0 && typeof mainStackView.currentItem.bannerViewHeight === "number") {
            AdMobHelper.showBannerView();
        } else {
            AdMobHelper.hideBannerView();
        }
    }

    onAdMobConsentChanged: {
        AppSettings.adMobConsent = adMobConsent;

        updateFeatures();
    }

    function updateFeatures() {
        if (adMobConsent === "PERSONALIZED" || adMobConsent === "NON_PERSONALIZED") {
            AdMobHelper.setPersonalization(adMobConsent === "PERSONALIZED");

            AdMobHelper.initAds();
        }

        if (mainStackView.depth > 0 && typeof mainStackView.currentItem.bannerViewHeight === "number") {
            AdMobHelper.showBannerView();
        } else {
            AdMobHelper.hideBannerView();
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
                    AdMobHelper.showBannerView();
                } else {
                    AdMobHelper.hideBannerView();
                }

                if (currentItem.allowInterstitial) {
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
        AppSettings.defaultBrushSize = Qt.binding(function() { return UtilScript.dp(UIHelper.screenDpi, 16); });

        adMobConsent = AppSettings.adMobConsent;

        updateFeatures();

        var component = Qt.createComponent("Core/ModeSelectionPage.qml");

        if (component.status === Component.Ready) {
            mainStackView.push(component);
        } else {
            console.log(component.errorString());
        }

        if (adMobConsent !== "PERSONALIZED" && adMobConsent !== "NON_PERSONALIZED") {
            adMobConsentDialog.open();
        }
    }
}
