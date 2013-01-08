import bb.cascades 1.0
import ImageEditor 1.0

Page {
    id: sketchPreviewPage

    property string imageFileName: ""

    function openImage(imageFile) {
        imageFileName = imageFile;

        activityIndicator.visible = true;
        activityIndicator.start();

        sketchPreviewGenerator.openImage(imageFile);
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
            
            property Page helpPage: null
            
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
                    SketchPreviewGenerator {
                        id: sketchPreviewGenerator
                        
                        onImageOpened: {
                            gaussianRadiusSlider.enabled = true;
                            applyButton.enabled          = true;
                        }
                        
                        onImageOpenFailed: {
                            activityIndicator.stop();
                            activityIndicator.visible = false;

                            gaussianRadiusSlider.enabled = false;
                            applyButton.enabled          = false;

                            MessageBox.showMessage(qsTr("Error"), qsTr("Could not open image"), qsTr("OK"));
                        }
                        
                        onNeedRepaint: {
                            activityIndicator.stop();
                            activityIndicator.visible = false;

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
            fromValue:           4
            toValue:             18
            value:               11
            enabled:             false

            layoutProperties: StackLayoutProperties {
                spaceQuota: -1
            }
            
            onValueChanged: {
                activityIndicator.visible = true;
                activityIndicator.start();
                
                sketchPreviewGenerator.radius = value;
            }
        }

        Button {
            id:                  applyButton
            horizontalAlignment: HorizontalAlignment.Center
            verticalAlignment:   VerticalAlignment.Bottom
            text:                qsTr("Apply")
            enabled:             false
            
            layoutProperties: StackLayoutProperties {
                spaceQuota: -1
            }
            
            onClicked: {
            }
        }
    }
}
