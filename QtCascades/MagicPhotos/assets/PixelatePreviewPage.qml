import bb.cascades 1.0
import bb.system 1.0
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

    Container {
        background: Color.Black

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

                attachedObjects: [
                    PixelatePreviewGenerator {
                        id: pixelatePreviewGenerator
                        
                        property int activityIndicatorUsageCounter: 0
                        
                        onImageOpened: {
                            pixDenomSlider.enabled = true;
                            applyButton.enabled    = true;
                        }

                        onImageOpenFailed: {
                            pixDenomSlider.enabled = false;
                            applyButton.enabled    = false;

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

                        onNeedRepaint: {
                            previewImageView.image = image;                            
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
                preferredWidth:      256
                preferredHeight:     256
                horizontalAlignment: HorizontalAlignment.Center
                verticalAlignment:   VerticalAlignment.Center
                visible:             false
            }
        }

        Slider {
            id:                  pixDenomSlider
            horizontalAlignment: HorizontalAlignment.Center 
            fromValue:           32
            toValue:             192
            value:               112
            enabled:             false

            onValueChanged: {
                pixelatePreviewGenerator.pixDenom = value;
            }

            layoutProperties: StackLayoutProperties {
                spaceQuota: -1
            }
        }

        Button {
            id:                  applyButton
            horizontalAlignment: HorizontalAlignment.Center
            verticalAlignment:   VerticalAlignment.Bottom
            text:                qsTr("Apply")
            enabled:             false
            
            onClicked: {
                var page = pixelatePageDefinition.createObject();

                navigationPane.push(page);

                page.openImage(imageFile, pixDenomSlider.value);
            }

            layoutProperties: StackLayoutProperties {
                spaceQuota: -1
            }
            
            attachedObjects: [
                ComponentDefinition {
                    id:     pixelatePageDefinition
                    source: "PixelatePage.qml"
                }
            ]
        }
    }
}
