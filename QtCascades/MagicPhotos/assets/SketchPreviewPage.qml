import bb.cascades 1.0
import ImageEditor 1.0

Page {
    id: sketchPreviewPage

    function openImage(imageFile) {
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

        Slider {
            id:                  gaussianRadiusSlider
            horizontalAlignment: HorizontalAlignment.Center 
            fromValue:           4
            toValue:             18
            value:               11

            layoutProperties: StackLayoutProperties {
                spaceQuota: -1
            }
            
            onValueChanged: {
                activityIndicator.visible = true;
                activityIndicator.start();
                
                sketchPreviewGenerator.radius = value;
            }
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
                        
                        onImageOpenFailed: {
                            activityIndicator.stop();
                            activityIndicator.visible = false;

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

        Button {
            id:                  applyButton
            horizontalAlignment: HorizontalAlignment.Center
            verticalAlignment:   VerticalAlignment.Bottom
            text:                qsTr("Apply")
            
            layoutProperties: StackLayoutProperties {
                spaceQuota: -1
            }
            
            onClicked: {
            }
        }
    }
}
