import bb.cascades 1.0
import FilePicker 1.0
import ImageEditor 1.0

Page {
    id: sketchPage

    function openImage(image_file, gaussian_radius) {
        activityIndicator.visible = true;
        activityIndicator.start();

        sketchEditor.radius = gaussian_radius;
        sketchEditor.openImage(image_file);
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
            id:                  undoActionItem
            title:               qsTr("Undo")
            imageSource:         "images/undo.png"
            ActionBar.placement: ActionBarPlacement.OnBar
            enabled:             false

            onTriggered: {
                sketchEditor.undo();
            }
        },
        ActionItem {
            id:                  saveActionItem
            title:               qsTr("Save")
            imageSource:         "images/save.png"
            ActionBar.placement: ActionBarPlacement.OnBar
            enabled:             false

            onTriggered: {
                saveFilePicker.open();
            }
            
            attachedObjects: [
                FilePicker {
                    id:             saveFilePicker
                    type:           FileType.Picture
                    mode:           FilePickerMode.Saver
                    allowOverwrite: true
                    title:          qsTr("Save Image")
                    
                    onFileSelected: {
                        sketchEditor.saveImage(selectedFiles[0]);
                    } 
                }
            ]
        },
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
        id:         sketchPageContainer
        background: Color.Black

        layout: StackLayout {
        }

        SegmentedControl {
            id:                  modeSegmentedControl
            horizontalAlignment: HorizontalAlignment.Center
            
            layoutProperties: StackLayoutProperties {
                spaceQuota: -1
            }

            onSelectedValueChanged: {
                if (selectedValue === SketchEditor.ModeScroll) {
                    imageScrollView.touchPropagationMode = TouchPropagationMode.Full;
                } else {
                    imageScrollView.touchPropagationMode = TouchPropagationMode.PassThrough;
                }
                
                sketchEditor.mode = selectedValue;
            }

            Option {
                id:          scrollModeOption
                value:       SketchEditor.ModeScroll
                imageSource: "images/mode_scroll.png"
            }

            Option {
                id:          originalModeOption
                value:       SketchEditor.ModeOriginal
                imageSource: "images/mode_original.png"
                enabled:     false
            }

            Option {
                id:          effectedModeOption
                value:       SketchEditor.ModeEffected
                imageSource: "images/mode_effected.png"
                enabled:     false
            }
        }

        Container {
            id:                  imageContainer
            preferredWidth:      sketchPageContainerLayoutUpdateHandler.layoutFrame.width
            horizontalAlignment: HorizontalAlignment.Center 
            background:          Color.Transparent

            layoutProperties: StackLayoutProperties {
                spaceQuota: 1
            }

            layout: DockLayout {
            }

            function showHelper(touch_x, touch_y) {
                if (modeSegmentedControl.selectedValue !== SketchEditor.ModeScroll) {
                    helperImageView.visible = true;

                    var local_x = imageScrollViewLayoutUpdateHandler.layoutFrame.x + touch_x * imageScrollView.contentScale - imageScrollView.viewableArea.x;
                    var local_y = imageScrollViewLayoutUpdateHandler.layoutFrame.y + touch_y * imageScrollView.contentScale - imageScrollView.viewableArea.y;
    
                    if (local_y < helperImageViewLayoutUpdateHandler.layoutFrame.height * 2) {
                        if (local_x < helperImageViewLayoutUpdateHandler.layoutFrame.width * 2) {
                            helperImageView.horizontalAlignment = HorizontalAlignment.Right;
                        } else if (local_x > imageContainerLayoutUpdateHandler.layoutFrame.width - helperImageViewLayoutUpdateHandler.layoutFrame.width * 2) {
                            helperImageView.horizontalAlignment = HorizontalAlignment.Left;
                        }
                    }
                } else {
                    helperImageView.visible = false;
                }
            }

            ScrollView {
                id:                   imageScrollView
                horizontalAlignment:  HorizontalAlignment.Center
                verticalAlignment:    VerticalAlignment.Center
                touchPropagationMode: TouchPropagationMode.Full
                
                scrollViewProperties {
                    scrollMode:         ScrollMode.Both
                    pinchToZoomEnabled: true
                    minContentScale:    1.0
                    maxContentScale:    4.0
                }            
                
                ImageView {
                    id:            imageView
                    scalingMethod: ScalingMethod.AspectFit 
                    
                    onTouch: {
                        if (event.touchType === TouchType.Down) {
                            imageContainer.showHelper(event.localX, event.localY);

                            sketchEditor.changeImageAt(true, event.localX, event.localY, imageScrollView.contentScale);
                        } else if (event.touchType === TouchType.Move) {
                            imageContainer.showHelper(event.localX, event.localY);

                            sketchEditor.changeImageAt(false, event.localX, event.localY, imageScrollView.contentScale);
                        } else {
                            helperImageView.visible = false;
                        }
                    }

                    attachedObjects: [
                        SketchEditor {
                            id:   sketchEditor
                            mode: SketchEditor.ModeScroll 
                            
                            onImageOpened: {
                                activityIndicator.stop();
                                activityIndicator.visible = false;
                                
                                saveActionItem.enabled = true;
                                
                                modeSegmentedControl.selectedOption = scrollModeOption;
                                
                                originalModeOption.enabled = true;
                                effectedModeOption.enabled = true;
                                
                                imageScrollView.resetViewableArea(ScrollAnimation.Default);
                            }
                            
                            onImageOpenFailed: {
                                activityIndicator.stop();
                                activityIndicator.visible = false;
                                
                                saveActionItem.enabled = false;
                                
                                modeSegmentedControl.selectedOption = scrollModeOption;
                                
                                originalModeOption.enabled = false;
                                effectedModeOption.enabled = false;
                                
                                imageScrollView.resetViewableArea(ScrollAnimation.Default);
                                
                                MessageBox.showMessage(qsTr("Error"), qsTr("Could not open image"), qsTr("OK"));
                            }
                            
                            onImageSaved: {
                                MessageBox.showMessage(qsTr("Info"), qsTr("Image saved successfully"), qsTr("OK"));
                            }
                            
                            onImageSaveFailed: {
                                MessageBox.showMessage(qsTr("Error"), qsTr("Could not save image"), qsTr("OK"));
                            }
                            
                            onUndoAvailabilityChanged: {
                                if (available) {
                                    undoActionItem.enabled = true;
                                } else {
                                    undoActionItem.enabled = false;
                                }
                            }
                            
                            onNeedImageRepaint: {
                                imageView.image = image;                            
                            }
                            
                            onNeedHelperRepaint: {
                                helperImageView.image = image;
                            }
                        }
                    ]
                }
                
                attachedObjects: [
                    LayoutUpdateHandler {
                        id: imageScrollViewLayoutUpdateHandler
                    }
                ]
            }
            
            ImageView {
                id:                  helperImageView
                horizontalAlignment: HorizontalAlignment.Left
                verticalAlignment:   VerticalAlignment.Top
                visible:             false
                
                attachedObjects: [
                    LayoutUpdateHandler {
                        id: helperImageViewLayoutUpdateHandler
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

            attachedObjects: [
                LayoutUpdateHandler {
                    id: imageContainerLayoutUpdateHandler
                }
            ]
        }

        attachedObjects: [
            LayoutUpdateHandler {
                id: sketchPageContainerLayoutUpdateHandler
            }
        ]
    }
}
