import bb.cascades 1.3
import bb.system 1.2
import ImageEditor 1.0

Page {
    id: cartoonPreviewPage

    property string imageFile: ""

    function openImage(image_file) {
        imageFile = image_file;

        cartoonPreviewGenerator.openImage(image_file);
    }

    paneProperties: NavigationPaneProperties {
        backButton: ActionItem {
            onTriggered: {
                navigationPane.pop();
            }
        }
    }

    actions: [
        ActionItem {
            id:                  applyActionItem
            title:               qsTr("Apply")
            imageSource:         "images/apply.png"
            ActionBar.placement: ActionBarPlacement.Signature
            enabled:             false
            
            onTriggered: {
                var page = cartoonPageDefinition.createObject();
                
                navigationPane.push(page);
                
                page.openImage(imageFile, gaussianRadiusSlider.value, thresholdSlider.value);
            }

            attachedObjects: [
                ComponentDefinition {
                    id:     cartoonPageDefinition
                    source: "CartoonPage.qml"
                }
            ]
        }
    ]

    Container {
        background:    Color.Black
        bottomPadding: ui.sdu(8)

        layout: StackLayout {
        }

        Container {
            horizontalAlignment: HorizontalAlignment.Center 
            background:          Color.Transparent

            layoutProperties: StackLayoutProperties {
                spaceQuota: 1
            }
            
            layout: DockLayout {
            }

            ImageView {
                id:                  previewImageView
                horizontalAlignment: HorizontalAlignment.Center 
                verticalAlignment:   VerticalAlignment.Center
                scalingMethod:       ScalingMethod.AspectFit
                accessibility.name:  qsTr("Resulting image preview")

                attachedObjects: [
                    CartoonPreviewGenerator {
                        id:      cartoonPreviewGenerator
                        preview: previewImageView
                        
                        property int activityIndicatorUsageCounter: 0
                        
                        onImageOpened: {
                            gaussianRadiusSlider.enabled = true;
                            thresholdSlider.enabled      = true;
                            applyActionItem.enabled      = true;
                        }

                        onImageOpenFailed: {
                            gaussianRadiusSlider.enabled = false;
                            thresholdSlider.enabled      = false;
                            applyActionItem.enabled      = false;

                            imageOpenFailedToast.show();
                        }
                        
                        onGenerationStarted: {
                            activityIndicatorUsageCounter = activityIndicatorUsageCounter + 1;
                            
                            if (activityIndicatorUsageCounter === 1) {
                                activityIndicator.visible = true;
                                activityIndicator.start();
                            }
                        }

                        onGenerationFinished: {
                            if (activityIndicatorUsageCounter === 1) {
                                activityIndicator.stop();
                                activityIndicator.visible = false;
                            }
                            
                            if (activityIndicatorUsageCounter > 0) {
                                activityIndicatorUsageCounter = activityIndicatorUsageCounter - 1;
                            }
                        }
                    },
                    SystemToast {
                        id:   imageOpenFailedToast
                        body: qsTr("Could not open image")
                    }
                ]
            }

            ActivityIndicator {
                id:                  activityIndicator
                preferredWidth:      ui.sdu(24)
                preferredHeight:     ui.sdu(24)
                horizontalAlignment: HorizontalAlignment.Center
                verticalAlignment:   VerticalAlignment.Center
                visible:             false
                accessibility.name:  qsTr("Activity indicator")
            }
        }

        Slider {
            id:                  gaussianRadiusSlider
            horizontalAlignment: HorizontalAlignment.Center 
            fromValue:           0
            toValue:             10
            value:               5
            enabled:             false
            accessibility.name:  qsTr("Gaussian radius slider")

            onValueChanged: {
                cartoonPreviewGenerator.radius = value;
            }

            layoutProperties: StackLayoutProperties {
                spaceQuota: -1
            }
        }

        Slider {
            id:                  thresholdSlider
            horizontalAlignment: HorizontalAlignment.Center
            fromValue:           32
            toValue:             128
            value:               80
            enabled:             false
            accessibility.name:  qsTr("Threshold slider")

            onValueChanged: {
                cartoonPreviewGenerator.threshold = value;
            }

            layoutProperties: StackLayoutProperties {
                spaceQuota: -1
            }
        }
    }
}
