import bb.cascades 1.0
import bb.system 1.0
import FilePicker 1.0
import CustomTimer 1.0
import ImageEditor 1.0

Page {
    id: cartoonPage

    function openImage(image_file, gaussian_radius, threshold) {
        activityIndicator.visible = true;
        activityIndicator.start();

        cartoonEditor.radius    = gaussian_radius;
        cartoonEditor.threshold = threshold;
        cartoonEditor.openImage(image_file);
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
            id:                  saveActionItem
            title:               qsTr("Save")
            imageSource:         "images/save.png"
            ActionBar.placement: ActionBarPlacement.OnBar
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
                        cartoonEditor.saveImage(selectedFiles[0]);
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
                cartoonEditor.undo();
            }
        },
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
        id:         cartoonPageContainer
        background: Color.Black

        layout: StackLayout {
        }

        SegmentedControl {
            id:                  modeSegmentedControl
            horizontalAlignment: HorizontalAlignment.Center
            
            onSelectedValueChanged: {
                if (selectedValue === CartoonEditor.ModeScroll) {
                    imageScrollView.touchPropagationMode = TouchPropagationMode.Full;
                } else {
                    imageScrollView.touchPropagationMode = TouchPropagationMode.PassThrough;
                }
                
                cartoonEditor.mode = selectedValue;
            }

            layoutProperties: StackLayoutProperties {
                spaceQuota: -1
            }

            Option {
                id:          scrollModeOption
                value:       CartoonEditor.ModeScroll
                imageSource: "images/mode_scroll.png"
            }

            Option {
                id:          originalModeOption
                value:       CartoonEditor.ModeOriginal
                imageSource: "images/mode_original.png"
                enabled:     false
            }

            Option {
                id:          effectedModeOption
                value:       CartoonEditor.ModeEffected
                imageSource: "images/mode_effected.png"
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
                if (modeSegmentedControl.selectedValue !== CartoonEditor.ModeScroll) {
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
                    maxContentScale:    8.0
                }            
                
                ImageView {
                    id:            imageView
                    scalingMethod: ScalingMethod.AspectFit 
                    
                    onTouch: {
                        if (event.touchType === TouchType.Down) {
                            imageContainer.showHelper(event.localX, event.localY);

                            cartoonEditor.changeImageAt(true, event.localX, event.localY, imageScrollView.contentScale);
                        } else if (event.touchType === TouchType.Move) {
                            imageContainer.showHelper(event.localX, event.localY);

                            cartoonEditor.changeImageAt(false, event.localX, event.localY, imageScrollView.contentScale);
                        } else {
                            helperImageView.visible = false;
                        }
                    }

                    attachedObjects: [
                        CartoonEditor {
                            id:   cartoonEditor
                            mode: CartoonEditor.ModeScroll 
                            
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
                            
                            onNeedImageRepaint: {
                                imageView.image = image;                            
                            }
                            
                            onNeedHelperRepaint: {
                                helperImageView.image = image;
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
