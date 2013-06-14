import bb.cascades 1.0
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
            id:                  helpActionItem
            title:               qsTr("Help")
            imageSource:         "images/help.png"
            ActionBar.placement: ActionBarPlacement.OnBar
            
            onTriggered: {
                navigationPane.push(helpPageDefinition.createObject());
            }

            attachedObjects: [
                ComponentDefinition {
                    id:     helpPageDefinition
                    source: "HelpPage.qml"
                }
            ]
        }
    ]
    
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
                    CartoonPreviewGenerator {
                        id: cartoonPreviewGenerator
                        
                        property int activityIndicatorUsageCounter: 0
                        
                        onImageOpened: {
                            gaussianRadiusSlider.enabled = true;
                            thresholdSlider.enabled      = true;
                            applyButton.enabled          = true;
                        }

                        onImageOpenFailed: {
                            gaussianRadiusSlider.enabled = false;
                            thresholdSlider.enabled      = false;
                            applyButton.enabled          = false;

                            MessageBox.showToast(qsTr("Could not open image"));
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
            id:                  gaussianRadiusSlider
            horizontalAlignment: HorizontalAlignment.Center 
            fromValue:           0
            toValue:             10
            value:               5
            enabled:             false

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

            onValueChanged: {
                cartoonPreviewGenerator.threshold = value;
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
                var page = cartoonPageDefinition.createObject();

                navigationPane.push(page);

                page.openImage(imageFile, gaussianRadiusSlider.value, thresholdSlider.value);
            }

            layoutProperties: StackLayoutProperties {
                spaceQuota: -1
            }
            
            attachedObjects: [
                ComponentDefinition {
                    id:     cartoonPageDefinition
                    source: "CartoonPage.qml"
                }
            ]
        }
    }
}
