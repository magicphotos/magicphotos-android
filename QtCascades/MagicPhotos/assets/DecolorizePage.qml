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
        }

        ScrollView {
            id:                   imageScrollView
            horizontalAlignment:  HorizontalAlignment.Center
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
                        decolorizeEditor.changeImageAt(true, event.localX, event.localY, imageScrollView.contentScale);
                    } else if (event.touchType === TouchType.Move) {
                        decolorizeEditor.changeImageAt(false, event.localX, event.localY, imageScrollView.contentScale);
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
                        
                        onNeedRepaint: {
                            imageView.image = image;                            
                        }
                    }
                ]
            }
        }
    }
}
