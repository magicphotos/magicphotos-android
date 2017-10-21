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

    property bool fullVersion: false

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
