import bb.cascades 1.0
import FilePicker 1.0
import ImageEditor 1.0

Page {
    id: recolorPage

    function openImage(image_file) {
        activityIndicator.visible = true;
        activityIndicator.start();

        recolorEditor.openImage(image_file);
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
                recolorEditor.undo();
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
                        recolorEditor.saveImage(selectedFiles[0]);
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
        id:         recolorPageContainer
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
                if (selectedValue === RecolorEditor.ModeScroll) {
                    imageScrollView.touchPropagationMode = TouchPropagationMode.Full;
                } else {
                    imageScrollView.touchPropagationMode = TouchPropagationMode.PassThrough;
                }
                if (selectedValue === RecolorEditor.ModeHueSelection) {
                    hueSelectionContainer.visible = true;
                } else {
                    hueSelectionContainer.visible = false;
                }

                recolorEditor.mode = selectedValue;
            }

            Option {
                id:          scrollModeOption
                value:       RecolorEditor.ModeScroll
                imageSource: "images/mode_scroll.png"
            }

            Option {
                id:          originalModeOption
                value:       RecolorEditor.ModeOriginal
                imageSource: "images/mode_original.png"
                enabled:     false
            }

            Option {
                id:          effectedModeOption
                value:       RecolorEditor.ModeEffected
                imageSource: "images/mode_effected.png"
                enabled:     false
            }

            Option {
                id:          hueSelectionModeOption
                value:       RecolorEditor.ModeHueSelection
                imageSource: "images/mode_hue_selection.png"
                enabled:     false
            }
        }

        Container {
            id:                  imageContainer
            preferredWidth:      recolorPageContainerLayoutUpdateHandler.layoutFrame.width
            horizontalAlignment: HorizontalAlignment.Center 
            background:          Color.Transparent

            layoutProperties: StackLayoutProperties {
                spaceQuota: 1
            }

            layout: DockLayout {
            }

            function showHelper(touch_x, touch_y) {
                if (modeSegmentedControl.selectedValue !== RecolorEditor.ModeScroll && modeSegmentedControl.selectedValue !== RecolorEditor.ModeHueSelection) {
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

                            recolorEditor.changeImageAt(true, event.localX, event.localY, imageScrollView.contentScale);
                        } else if (event.touchType === TouchType.Move) {
                            imageContainer.showHelper(event.localX, event.localY);

                            recolorEditor.changeImageAt(false, event.localX, event.localY, imageScrollView.contentScale);
                        } else {
                            helperImageView.visible = false;
                        }
                    }

                    attachedObjects: [
                        RecolorEditor {
                            id:   recolorEditor
                            mode: RecolorEditor.ModeScroll
                            hue:  180 

                            onImageOpened: {
                                activityIndicator.stop();
                                activityIndicator.visible = false;
                                
                                saveActionItem.enabled = true;
                                
                                modeSegmentedControl.selectedOption = scrollModeOption;
                                
                                originalModeOption.enabled     = true;
                                effectedModeOption.enabled     = true;
                                hueSelectionModeOption.enabled = true;
                                
                                imageScrollView.resetViewableArea(ScrollAnimation.Default);
                            }

                            onImageOpenFailed: {
                                activityIndicator.stop();
                                activityIndicator.visible = false;
                                
                                saveActionItem.enabled = false;
                                
                                modeSegmentedControl.selectedOption = scrollModeOption;
                                
                                originalModeOption.enabled     = false;
                                effectedModeOption.enabled     = false;
                                hueSelectionModeOption.enabled = false;
                                
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
            
            Container {
                id:                  hueSelectionContainer
                horizontalAlignment: HorizontalAlignment.Right
                verticalAlignment:   VerticalAlignment.Center
                visible:             false
                
                layout: AbsoluteLayout {
                }

                onVisibleChanged: {
                    if (visible) {
                        hueSliderImageView.layoutProperties.positionY = Math.max(0, Math.min(hueBarImageView.preferredHeight - hueSliderImageView.preferredHeight, recolorEditor.hue));
                    }
                }
                
                ImageView {
                    id:              hueBarImageView
                    preferredWidth:  128
                    preferredHeight: 360
                    minWidth:        preferredWidth
                    minHeight:       preferredHeight
                    maxWidth:        preferredWidth
                    maxHeight:       preferredHeight
                    imageSource:     "images/hue_bar.png"
                    
                    layoutProperties: AbsoluteLayoutProperties {
                        positionX: 0
                        positionY: 0
                    }
                    
                    onTouch: {
                        if (event.touchType === TouchType.Down || event.touchType === TouchType.Move) {
                            hueSliderImageView.layoutProperties.positionY = Math.max(0, Math.min(hueBarImageView.preferredHeight - hueSliderImageView.preferredHeight, event.localY));

                            recolorEditor.hue = Math.max(0, Math.min(360, event.localY));
                        }
                    }  
                }
                
                ImageView {
                    id:                   hueSliderImageView
                    preferredWidth:       128
                    preferredHeight:      24
                    minWidth:             preferredWidth
                    minHeight:            preferredHeight
                    maxWidth:             preferredWidth
                    maxHeight:            preferredHeight
                    imageSource:          "images/hue_slider.png"
                    touchPropagationMode: TouchPropagationMode.PassThrough

                    layoutProperties: AbsoluteLayoutProperties {
                        positionX: 0
                        positionY: 0
                    }  
                }
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
                id: recolorPageContainerLayoutUpdateHandler
            }
        ]
    }
}
