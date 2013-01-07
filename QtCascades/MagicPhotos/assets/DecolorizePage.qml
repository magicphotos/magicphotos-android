import bb.cascades 1.0
import FilePicker 1.0
import ImageEditor 1.0

Page {
    id: decolorizePage

    function openImage(imageFile) {
        decolorizeEditor.openImage(imageFile);
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
                decolorizeEditor.undo();
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
                        decolorizeEditor.saveImage(selectedFiles[0]);
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
        id:         decolorizePageContainer
        background: Color.Black

        layout: StackLayout {
        }

        SegmentedControl {
            id:                  modeSegmentedControl
            horizontalAlignment: HorizontalAlignment.Center
            
            onSelectedValueChanged: {
                if (selectedValue === DecolorizeEditor.ModeScroll) {
                    imageScrollView.touchPropagationMode = TouchPropagationMode.Full;
                } else {
                    imageScrollView.touchPropagationMode = TouchPropagationMode.PassThrough;
                }
                
                decolorizeEditor.mode = selectedValue;
            }

            Option {
                id:          scrollModeOption
                value:       DecolorizeEditor.ModeScroll
                imageSource: "images/mode_scroll.png"
            }

            Option {
                id:          originalModeOption
                value:       DecolorizeEditor.ModeOriginal
                imageSource: "images/mode_original.png"
                enabled:     false
            }

            Option {
                id:          effectedModeOption
                value:       DecolorizeEditor.ModeEffected
                imageSource: "images/mode_effected.png"
                enabled:     false
            }

            attachedObjects: [
                LayoutUpdateHandler {
                    id: modeSegmentedControlLayoutUpdateHandler
                }
            ]
        }

        Container {
            id:                  imageContainer
            preferredWidth:      decolorizePageContainerLayoutUpdateHandler.layoutFrame.width
            preferredHeight:     decolorizePageContainerLayoutUpdateHandler.layoutFrame.height - modeSegmentedControlLayoutUpdateHandler.layoutFrame.height
            maxWidth:            preferredWidth
            maxHeight:           preferredHeight
            horizontalAlignment: HorizontalAlignment.Center 
            background:          Color.Transparent
            
            layout: DockLayout {
            }

            function showHelper(touch_x, touch_y) {
                if (modeSegmentedControl.selectedValue !== DecolorizeEditor.ModeScroll) {
                    helperImageContainer.visible = true;

                    var local_x = imageScrollViewLayoutUpdateHandler.layoutFrame.x + touch_x * imageScrollView.contentScale - imageScrollView.viewableArea.x;
                    var local_y = imageScrollViewLayoutUpdateHandler.layoutFrame.y + touch_y * imageScrollView.contentScale - imageScrollView.viewableArea.y;
    
                    if (local_y < helperImageContainerLayoutUpdateHandler.layoutFrame.height * 2) {
                        if (local_x < helperImageContainerLayoutUpdateHandler.layoutFrame.width * 2) {
                            helperImageContainer.horizontalAlignment = HorizontalAlignment.Right;
                        } else if (local_x > imageContainerLayoutUpdateHandler.layoutFrame.width - helperImageContainerLayoutUpdateHandler.layoutFrame.width * 2) {
                            helperImageContainer.horizontalAlignment = HorizontalAlignment.Left;
                        }
                    }
                } else {
                    helperImageContainer.visible = false;
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

                            decolorizeEditor.changeImageAt(true, event.localX, event.localY, imageScrollView.contentScale);
                        } else if (event.touchType === TouchType.Move) {
                            imageContainer.showHelper(event.localX, event.localY);

                            decolorizeEditor.changeImageAt(false, event.localX, event.localY, imageScrollView.contentScale);
                        } else {
                            helperImageContainer.visible = false;
                        }
                    }

                    attachedObjects: [
                        DecolorizeEditor {
                            id:   decolorizeEditor
                            mode: DecolorizeEditor.ModeScroll 
                            
                            onImageOpened: {
                                saveActionItem.enabled = true;
                                
                                modeSegmentedControl.selectedOption = scrollModeOption;
                                
                                originalModeOption.enabled = true;
                                effectedModeOption.enabled = true;
                                
                                imageScrollView.resetViewableArea(ScrollAnimation.Default);
                            }
                            
                            onImageOpenFailed: {
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
            
            Container {
                id:                  helperImageContainer
                background:          Color.Transparent
                horizontalAlignment: HorizontalAlignment.Left
                verticalAlignment:   VerticalAlignment.Top
                visible:             false
                
                ImageView {
                    id: helperImageView
                } 

                attachedObjects: [
                    LayoutUpdateHandler {
                        id: helperImageContainerLayoutUpdateHandler
                    }
                ]
            }

            attachedObjects: [
                LayoutUpdateHandler {
                    id: imageContainerLayoutUpdateHandler
                }
            ]
        }

        attachedObjects: [
            LayoutUpdateHandler {
                id: decolorizePageContainerLayoutUpdateHandler
            }
        ]
    }
}
