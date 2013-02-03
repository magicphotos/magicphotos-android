import bb.cascades 1.0
import FilePicker 1.0
import ImageEditor 1.0

Page {
    id: retouchPage

    function openImage(image_file) {
        activityIndicator.visible = true;
        activityIndicator.start();

        retouchEditor.openImage(image_file);
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
                retouchEditor.undo();
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
                        retouchEditor.saveImage(selectedFiles[0]);
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
        id:         retouchPageContainer
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
                if (selectedValue === RetouchEditor.ModeScroll) {
                    imageScrollView.touchPropagationMode = TouchPropagationMode.Full;
                } else {
                    imageScrollView.touchPropagationMode = TouchPropagationMode.PassThrough;
                }

                retouchEditor.mode = selectedValue;
            }

            Option {
                id:          scrollModeOption
                value:       RetouchEditor.ModeScroll
                imageSource: "images/mode_scroll.png"
            }

            Option {
                id:          samplingPointModeOption
                value:       RetouchEditor.ModeSamplingPoint
                imageSource: "images/mode_sampling_point.png"
                enabled:     false
            }

            Option {
                id:          cloneModeOption
                value:       RetouchEditor.ModeClone
                imageSource: "images/mode_clone.png"
                enabled:     false
            }
        }

        Container {
            id:                  imageContainer
            preferredWidth:      65535
            horizontalAlignment: HorizontalAlignment.Center 
            background:          Color.Transparent

            layoutProperties: StackLayoutProperties {
                spaceQuota: 1
            }

            layout: DockLayout {
            }

            function showHelper(touch_x, touch_y) {
                if (modeSegmentedControl.selectedValue !== RetouchEditor.ModeScroll) {
                    helperImageView.visible = true;

                    var local_x = imageScrollViewLayoutUpdateHandler.layoutFrame.x + (imageViewContainerLayoutUpdateHandler.layoutFrame.x + touch_x) * imageScrollView.contentScale - imageScrollView.viewableArea.x;
                    var local_y = imageScrollViewLayoutUpdateHandler.layoutFrame.y + (imageViewContainerLayoutUpdateHandler.layoutFrame.y + touch_y) * imageScrollView.contentScale - imageScrollView.viewableArea.y;
    
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
                
                onContentScaleChanged: {
                    if (retouchEditor.samplingPointValid) {
                        var sampling_point_x = Math.max(samplingPointImageView.preferredWidth  / 2, Math.min(retouchEditor.imageWidth  - samplingPointImageView.preferredWidth  / 2, retouchEditor.samplingPointX));
                        var sampling_point_y = Math.max(samplingPointImageView.preferredHeight / 2, Math.min(retouchEditor.imageHeight - samplingPointImageView.preferredHeight / 2, retouchEditor.samplingPointY));

                        retouchEditor.samplingPointX = sampling_point_x;
                        retouchEditor.samplingPointY = sampling_point_y;

                        samplingPointImageView.layoutProperties.positionX = sampling_point_x - samplingPointImageView.preferredWidth  / 2;
                        samplingPointImageView.layoutProperties.positionY = sampling_point_y - samplingPointImageView.preferredHeight / 2;
                    }
                }
                
                Container {
                    id: imageViewContainer
                    
                    layout: AbsoluteLayout {
                    }

                    ImageView {
                        id:            imageView
                        scalingMethod: ScalingMethod.AspectFit 

                        layoutProperties: AbsoluteLayoutProperties {
                            positionX: 0
                            positionY: 0
                        }

                        property int initial_sampling_point_x: 0
                        property int initial_sampling_point_y: 0
                        property int initial_touch_point_x:    0
                        property int initial_touch_point_y:    0

                        onTouch: {
                            if (modeSegmentedControl.selectedValue === RetouchEditor.ModeSamplingPoint) {
                                if (event.touchType === TouchType.Down || event.touchType === TouchType.Move) {
                                    var sampling_point_x = Math.max(samplingPointImageView.preferredWidth  / 2, Math.min(retouchEditor.imageWidth  - samplingPointImageView.preferredWidth  / 2, event.localX));
                                    var sampling_point_y = Math.max(samplingPointImageView.preferredHeight / 2, Math.min(retouchEditor.imageHeight - samplingPointImageView.preferredHeight / 2, event.localY));

                                    retouchEditor.samplingPointValid = true;
                                    retouchEditor.samplingPointX     = sampling_point_x;
                                    retouchEditor.samplingPointY     = sampling_point_y;

                                    samplingPointImageView.visible                    = true;
                                    samplingPointImageView.layoutProperties.positionX = sampling_point_x - samplingPointImageView.preferredWidth  / 2;
                                    samplingPointImageView.layoutProperties.positionY = sampling_point_y - samplingPointImageView.preferredHeight / 2;

                                    imageContainer.showHelper(event.localX, event.localY);

                                    retouchEditor.updateHelperAt(sampling_point_x, sampling_point_y, imageScrollView.contentScale);
                                } else {
                                    helperImageView.visible = false;
                                }
                            } else if (modeSegmentedControl.selectedValue === RetouchEditor.ModeClone) {
                                if (retouchEditor.samplingPointValid) {
                                    if (event.touchType === TouchType.Down) {
                                        initial_sampling_point_x = retouchEditor.samplingPointX;
                                        initial_sampling_point_y = retouchEditor.samplingPointY;
                                        initial_touch_point_x    = event.localX;
                                        initial_touch_point_y    = event.localY;
                                        
                                        imageContainer.showHelper(event.localX, event.localY);
                                        
                                        retouchEditor.changeImageAt(true, event.localX, event.localY, imageScrollView.contentScale);
                                    } else if (event.touchType === TouchType.Move) {
                                        var sampling_point_x = Math.max(samplingPointImageView.preferredWidth  / 2, Math.min(retouchEditor.imageWidth  - samplingPointImageView.preferredWidth  / 2, initial_sampling_point_x + event.localX - initial_touch_point_x));
                                        var sampling_point_y = Math.max(samplingPointImageView.preferredHeight / 2, Math.min(retouchEditor.imageHeight - samplingPointImageView.preferredHeight / 2, initial_sampling_point_y + event.localY - initial_touch_point_y));

                                        retouchEditor.samplingPointX = sampling_point_x;
                                        retouchEditor.samplingPointY = sampling_point_y;

                                        samplingPointImageView.layoutProperties.positionX = sampling_point_x - samplingPointImageView.preferredWidth  / 2;
                                        samplingPointImageView.layoutProperties.positionY = sampling_point_y - samplingPointImageView.preferredHeight / 2;

                                        imageContainer.showHelper(event.localX, event.localY);

                                        retouchEditor.changeImageAt(false, event.localX, event.localY, imageScrollView.contentScale);
                                    } else {
                                        helperImageView.visible = false;
                                    }
                                }
                            }
                        }

                        attachedObjects: [
                            RetouchEditor {
                                id:   retouchEditor
                                mode: RetouchEditor.ModeScroll
                                
                                onImageOpened: {
                                    activityIndicator.stop();
                                    activityIndicator.visible = false;
                                    
                                    saveActionItem.enabled = true;
                                    
                                    modeSegmentedControl.selectedOption = scrollModeOption;
                                    
                                    samplingPointModeOption.enabled = true;
                                    cloneModeOption.enabled         = true;

                                    samplingPointValid             = false;
                                    samplingPointImageView.visible = false;
                                    
                                    imageScrollView.resetViewableArea(ScrollAnimation.Default);
                                }
                                
                                onImageOpenFailed: {
                                    activityIndicator.stop();
                                    activityIndicator.visible = false;
                                    
                                    saveActionItem.enabled = false;
                                    
                                    modeSegmentedControl.selectedOption = scrollModeOption;
                                    
                                    samplingPointModeOption.enabled = false;
                                    cloneModeOption.enabled         = false;

                                    samplingPointValid             = false;
                                    samplingPointImageView.visible = false;

                                    imageScrollView.resetViewableArea(ScrollAnimation.Default);
                                    
                                    MessageBox.showToast(qsTr("Could not open image"));
                                }
                                
                                onImageSaved: {
                                    MessageBox.showToast(qsTr("Image saved successfully"));
                                }
                                
                                onImageSaveFailed: {
                                    MessageBox.showToast(qsTr("Could not save image"));
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
                    
                    ImageView {
                        id:                   samplingPointImageView
                        preferredWidth:       64 / imageScrollView.contentScale
                        preferredHeight:      64 / imageScrollView.contentScale
                        minWidth:             preferredWidth
                        minHeight:            preferredHeight
                        maxWidth:             preferredWidth
                        maxHeight:            preferredHeight
                        imageSource:          "images/sampling_point.png"
                        touchPropagationMode: TouchPropagationMode.PassThrough
                        overlapTouchPolicy:   OverlapTouchPolicy.Allow
                        visible:              false
                        
                        layoutProperties: AbsoluteLayoutProperties {
                            positionX: 0
                            positionY: 0
                        }
                    }

                    attachedObjects: [
                        LayoutUpdateHandler {
                            id: imageViewContainerLayoutUpdateHandler
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
    }
}
