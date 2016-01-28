import bb.cascades 1.3
import bb.system 1.2
import FilePicker 1.0
import CustomTimer 1.0
import ImageEditor 1.0

Page {
    id:         pixelatePage
    objectName: "editorPage"

    function openImage(image_file, pix_denom) {
        activityIndicator.visible = true;
        activityIndicator.start();

        pixelateEditor.pixDenom = pix_denom;
        pixelateEditor.openImage(image_file);
    }

    function updateEditorParameters() {
        pixelateEditor.brushSize       = AppSettings.brushSize;
        pixelateEditor.brushOpacity    = AppSettings.brushOpacity;
        pixelateEditor.resolutionLimit = AppSettings.imageResolutionLimit;
    }
    
    onCreationCompleted: {
        updateEditorParameters();
    }
    
    paneProperties: NavigationPaneProperties {
        backButton: ActionItem {
            onTriggered: {
                if (pixelateEditor.changed) {
                    backDialog.show();
                } else {
                    navigationPane.pop();
                }
            }
            
            attachedObjects: [
                SystemDialog {
                    id:    backDialog
                    title: qsTr("Warning")
                    body:  qsTr("Are you sure? Current image is not saved and will be lost.")
                    
                    onFinished: {
                        if (result === SystemUiResult.ConfirmButtonSelection) {
                            navigationPane.pop();
                        }
                    }
                }
            ]
        }
    }

    actions: [
        ActionItem {
            id:                  saveActionItem
            title:               qsTr("Save")
            imageSource:         "images/save.png"
            ActionBar.placement: ActionBarPlacement.Signature
            enabled:             false

            onTriggered: {
                if (TrialManager.trialMode) {
                    trialModeDialog.show();
                } else {
                    saveFilePicker.open();
                }
            }
            
            attachedObjects: [
                FilePicker {
                    id:             saveFilePicker
                    type:           FileType.Picture
                    mode:           FilePickerMode.Saver
                    allowOverwrite: true
                    title:          qsTr("Save Image")
                    
                    onFileSelected: {
                        pixelateEditor.saveImage(selectedFiles[0]);
                    } 
                },
                SystemDialog {
                    id:    trialModeDialog
                    title: qsTr("Info")
                    body:  qsTr("The save function is available in the full version only. Do you want to purchase full version now?")
                    
                    onFinished: {
                        if (result === SystemUiResult.ConfirmButtonSelection) {
                            appWorldFullInvocation.trigger("bb.action.OPEN");
                        }
                    }
                },
                SystemDialog {
                    id:                  requestFeedbackDialog
                    title:               qsTr("Info")
                    body:                qsTr("If you like this app, please take a moment to provide a feedback and rate it. Do you want to provide a feedback?")
                    confirmButton.label: qsTr("Yes")
                    cancelButton.label:  qsTr("Later")
                    customButton.label:  qsTr("Never")

                    onFinished: {
                        if (result === SystemUiResult.ConfirmButtonSelection) {
                            appWorldFullInvocation.trigger("bb.action.OPEN");

                            AppSettings.requestFeedback = false;
                        } else if (result === SystemUiResult.CancelButtonSelection) {
                            AppSettings.lastFeedbackRequestLaunchNumber = AppSettings.launchNumber;
                        } else if (result === SystemUiResult.CustomButtonSelection) {
                            AppSettings.requestFeedback = false;
                        }
                    }
                },
                CustomTimer {
                    id:         requestFeedbackTimer
                    singleShot: true
                    interval:   1000

                    onTimeout: {
                        if (AppSettings.requestFeedback && AppSettings.launchNumber > 1 && AppSettings.lastFeedbackRequestLaunchNumber !== AppSettings.launchNumber) {
                            requestFeedbackDialog.show();
                        }
                    }
                },
                Invocation {
                    id: appWorldFullInvocation

                    query: InvokeQuery {
                        mimeType: "application/x-bb-appworld"
                        uri:      "appworld://content/20356189"
                    }
                }
            ]
        },
        ActionItem {
            id:                  undoActionItem
            title:               qsTr("Undo")
            imageSource:         "images/undo.png"
            ActionBar.placement: ActionBarPlacement.OnBar
            enabled:             false

            onTriggered: {
                pixelateEditor.undo();
            }
        }
    ]
    
    Container {
        id:         pixelatePageContainer
        background: Color.Black
        topPadding: ui.sdu(2)

        layout: StackLayout {
        }

        SegmentedControl {
            id:                  modeSegmentedControl
            horizontalAlignment: HorizontalAlignment.Center
            accessibility.name:  qsTr("Editor modes")
            
            onSelectedValueChanged: {
                if (selectedValue === PixelateEditor.ModeScroll) {
                    imageScrollView.touchPropagationMode = TouchPropagationMode.Full;
                } else {
                    imageScrollView.touchPropagationMode = TouchPropagationMode.PassThrough;
                }
                
                pixelateEditor.mode = selectedValue;
            }

            layoutProperties: StackLayoutProperties {
                spaceQuota: -1
            }

            Option {
                id:          scrollModeOption
                value:       PixelateEditor.ModeScroll
                imageSource: "images/mode_scroll.png"
            }

            Option {
                id:          originalModeOption
                value:       PixelateEditor.ModeOriginal
                imageSource: "images/mode_original.png"
                enabled:     false
            }

            Option {
                id:          effectedModeOption
                value:       PixelateEditor.ModeEffected
                imageSource: "images/mode_effected.png"
                enabled:     false
            }
        }

        Container {
            id:                  imageContainer
            preferredWidth:      ui.px(65535)
            horizontalAlignment: HorizontalAlignment.Center 
            background:          Color.Transparent

            layoutProperties: StackLayoutProperties {
                spaceQuota: 1
            }

            layout: DockLayout {
            }

            function showHelper(touch_x, touch_y) {
                if (modeSegmentedControl.selectedValue !== PixelateEditor.ModeScroll) {
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
                accessibility.name:   qsTr("Image editor")
                
                scrollViewProperties {
                    scrollMode:         ScrollMode.Both
                    pinchToZoomEnabled: true
                    minContentScale:    0.5
                    maxContentScale:    8.0
                }            
                
                Container {
                    id:              fragmentsContainer
                    preferredWidth:  ui.px(0)
                    preferredHeight: ui.px(0)
                    
                    layout: AbsoluteLayout {
                    }
                    
                    onTouch: {
                        if (event.touchType === TouchType.Down) {
                            imageContainer.showHelper(event.localX, event.localY);

                            pixelateEditor.changeImageAt(true, event.localX, event.localY);
                        } else if (event.touchType === TouchType.Move) {
                            imageContainer.showHelper(event.localX, event.localY);

                            pixelateEditor.changeImageAt(false, event.localX, event.localY);
                        } else {
                            helperImageView.visible = false;
                        }
                    }

                    attachedObjects: [
                        PixelateEditor {
                            id:         pixelateEditor
                            mode:       PixelateEditor.ModeScroll 
                            scale:      imageScrollView.contentScale
                            helper:     helperImageView
                            helperSize: ui.sdu(20)
                            
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

                                imageOpenFailedToast.show();
                            }
                            
                            onImageSaved: {
                                imageSavedToast.show();

                                requestFeedbackTimer.start();
                            }
                            
                            onImageSaveFailed: {
                                imageSaveFailedToast.show();
                            }
                            
                            onUndoAvailabilityChanged: {
                                if (available) {
                                    undoActionItem.enabled = true;
                                } else {
                                    undoActionItem.enabled = false;
                                }
                            }
                            
                            onPrepareFragments: {
                                var fragments = getFragments();
                                
                                for (var i = 0; i < fragments.length; i++) {
                                    delFragment(fragments[i].posX, fragments[i].posY);
                                    
                                    fragmentsContainer.remove(fragments[i]);
                                    
                                    fragments[i].destroy();
                                }
                                
                                fragmentsContainer.preferredWidth  = imageWidth;
                                fragmentsContainer.preferredHeight = imageHeight;
                                
                                for (var x = 0; x < imageWidth;) {
                                    var fragment_width = Math.max(0, Math.min(fragmentSize, imageWidth - x));
                                    
                                    for (var y = 0; y < imageHeight;) {
                                        var fragment_height = Math.max(0, Math.min(fragmentSize, imageHeight - y));
                                        
                                        var fragment = fragmentImageViewDefinition.createObject();
                                        
                                        fragmentsContainer.add(fragment);
                                        
                                        fragment.posX            = x;
                                        fragment.posY            = y;
                                        fragment.preferredWidth  = fragment_width;
                                        fragment.preferredHeight = fragment_height;
                                        
                                        addFragment(x, y, fragment);
                                        
                                        y = y + fragment_height;
                                    }
                                    
                                    x = x + fragment_width;
                                }
                            }
                        },
                        ComponentDefinition {
                            id: fragmentImageViewDefinition
                            
                            ImageView {
                                scalingMethod:      ScalingMethod.AspectFit
                                accessibility.name: qsTr("Image fragment")
                                preferredWidth:     ui.px(0)
                                preferredHeight:    ui.px(0)
                                minWidth:           preferredWidth
                                minHeight:          preferredHeight
                                maxWidth:           preferredWidth
                                maxHeight:          preferredHeight
                                
                                property int posX: ui.px(0)
                                property int posY: ui.px(0)
                                
                                layoutProperties: AbsoluteLayoutProperties {
                                    positionX: posX
                                    positionY: posY
                                }
                            }
                        },
                        SystemToast {
                            id:   imageOpenFailedToast
                            body: qsTr("Could not open image")
                        },
                        SystemToast {
                            id:   imageSavedToast
                            body: qsTr("Image saved successfully")
                        },
                        SystemToast {
                            id:   imageSaveFailedToast
                            body: qsTr("Could not save image")
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
                accessibility.name:  qsTr("Helper image")
                
                attachedObjects: [
                    LayoutUpdateHandler {
                        id: helperImageViewLayoutUpdateHandler
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

            attachedObjects: [
                LayoutUpdateHandler {
                    id: imageContainerLayoutUpdateHandler
                }
            ]
        }
    }
}
