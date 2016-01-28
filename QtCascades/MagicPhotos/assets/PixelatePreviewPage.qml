import bb.cascades 1.3
import bb.system 1.2
import ImageEditor 1.0

Page {
    id: pixelatePreviewPage

    property string imageFile: ""

    function openImage(image_file) {
        imageFile = image_file;

        pixelatePreviewGenerator.openImage(image_file);
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
                var page = pixelatePageDefinition.createObject();
                
                navigationPane.push(page);
                
                page.openImage(imageFile, pixDenomSlider.value);
            }

            attachedObjects: [
                ComponentDefinition {
                    id:     pixelatePageDefinition
                    source: "PixelatePage.qml"
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
                    PixelatePreviewGenerator {
                        id:      pixelatePreviewGenerator
                        preview: previewImageView
                        
                        property int activityIndicatorUsageCounter: 0
                        
                        onImageOpened: {
                            pixDenomSlider.enabled  = true;
                            applyActionItem.enabled = true;
                        }

                        onImageOpenFailed: {
                            pixDenomSlider.enabled  = false;
                            applyActionItem.enabled = false;

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
            id:                  pixDenomSlider
            horizontalAlignment: HorizontalAlignment.Center 
            fromValue:           32
            toValue:             192
            value:               112
            enabled:             false
            accessibility.name:  qsTr("Pixelate denominator slider")

            onValueChanged: {
                pixelatePreviewGenerator.pixDenom = value;
            }

            layoutProperties: StackLayoutProperties {
                spaceQuota: -1
            }
        }
    }
}
