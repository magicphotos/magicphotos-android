import bb.cascades 1.3
import bb.system 1.2
import FilePicker 1.0
import CustomTimer 1.0
import ImageEditor 1.0

Page {
    id:         retouchPage
    objectName: "editorPage"

    function openImage(image_file) {
        activityIndicator.visible = true;
        activityIndicator.start();

        retouchEditor.openImage(image_file);
    }

    function updateEditorParameters() {
        retouchEditor.brushSize       = AppSettings.brushSize;
        retouchEditor.brushOpacity    = AppSettings.brushOpacity;
        retouchEditor.resolutionLimit = AppSettings.imageResolutionLimit;
    }
    
    onCreationCompleted: {
        updateEditorParameters();
    }
    
    paneProperties: NavigationPaneProperties {
        backButton: ActionItem {
            onTriggered: {
                if (retouchEditor.changed) {
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
                        retouchEditor.saveImage(selectedFiles[0]);
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
                retouchEditor.undo();
            }
        }
    ]
    
    Container {
        id:         retouchPageContainer
        background: Color.Black
        topPadding: ui.sdu(2)

        layout: StackLayout {
        }

        SegmentedControl {
            id:                  modeSegmentedControl
            horizontalAlignment: HorizontalAlignment.Center
            accessibility.name:  qsTr("Editor modes")
            
            onSelectedValueChanged: {
                if (selectedValue === RetouchEditor.ModeScroll) {
                    imageScrollView.touchPropagationMode = TouchPropagationMode.Full;
                } else {
                    imageScrollView.touchPropagationMode = TouchPropagationMode.PassThrough;
                }

                retouchEditor.mode = selectedValue;
            }

            layoutProperties: StackLayoutProperties {
                spaceQuota: -1
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

            Option {
                id:          blurModeOption
                value:       RetouchEditor.ModeBlur
                imageSource: "images/mode_blur.png"
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
                if (modeSegmentedControl.selectedValue !== RetouchEditor.ModeScroll) {
                    helperImageView.visible = true;

                    var local_x = imageScrollViewLayoutUpdateHandler.layoutFrame.x + (fragmentsContainerLayoutUpdateHandler.layoutFrame.x + touch_x) * imageScrollView.contentScale - imageScrollView.viewableArea.x;
                    var local_y = imageScrollViewLayoutUpdateHandler.layoutFrame.y + (fragmentsContainerLayoutUpdateHandler.layoutFrame.y + touch_y) * imageScrollView.contentScale - imageScrollView.viewableArea.y;

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
                
                scrollViewProperties {
                    scrollMode:         ScrollMode.Both
                    pinchToZoomEnabled: true
                    minContentScale:    0.5
                    maxContentScale:    8.0
                }            
                
                Container {
                    layout: AbsoluteLayout {
                    }

                    Container {
                        id:              fragmentsContainer
                        preferredWidth:  ui.px(0)
                        preferredHeight: ui.px(0)

                        layout: AbsoluteLayout {
                        }

                        property int initialSamplingPointX: 0
                        property int initialSamplingPointY: 0
                        property int initialTouchPointX:    0
                        property int initialTouchPointY:    0

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

                                    retouchEditor.updateHelperAt(sampling_point_x, sampling_point_y);
                                } else {
                                    helperImageView.visible = false;
                                }
                            } else if (modeSegmentedControl.selectedValue === RetouchEditor.ModeClone) {
                                if (retouchEditor.samplingPointValid) {
                                    if (event.touchType === TouchType.Down) {
                                        initialSamplingPointX = retouchEditor.samplingPointX;
                                        initialSamplingPointY = retouchEditor.samplingPointY;
                                        initialTouchPointX    = event.localX;
                                        initialTouchPointY    = event.localY;
                                        
                                        imageContainer.showHelper(event.localX, event.localY);
                                        
                                        retouchEditor.changeImageAt(true, event.localX, event.localY);
                                    } else if (event.touchType === TouchType.Move) {
                                        var sampling_point_x = Math.max(samplingPointImageView.preferredWidth  / 2, Math.min(retouchEditor.imageWidth  - samplingPointImageView.preferredWidth  / 2, initialSamplingPointX + event.localX - initialTouchPointX));
                                        var sampling_point_y = Math.max(samplingPointImageView.preferredHeight / 2, Math.min(retouchEditor.imageHeight - samplingPointImageView.preferredHeight / 2, initialSamplingPointY + event.localY - initialTouchPointY));

                                        retouchEditor.samplingPointX = sampling_point_x;
                                        retouchEditor.samplingPointY = sampling_point_y;

                                        samplingPointImageView.layoutProperties.positionX = sampling_point_x - samplingPointImageView.preferredWidth  / 2;
                                        samplingPointImageView.layoutProperties.positionY = sampling_point_y - samplingPointImageView.preferredHeight / 2;

                                        imageContainer.showHelper(event.localX, event.localY);

                                        retouchEditor.changeImageAt(false, event.localX, event.localY);
                                    } else {
                                        helperImageView.visible = false;
                                    }
                                }
                            } else if (modeSegmentedControl.selectedValue === RetouchEditor.ModeBlur) {
                                if (event.touchType === TouchType.Down) {
                                    imageContainer.showHelper(event.localX, event.localY);
                                    
                                    retouchEditor.changeImageAt(true, event.localX, event.localY);
                                    
                                    retouchEditor.lastBlurPointValid = true;
                                    retouchEditor.lastBlurPointX     = event.localX;
                                    retouchEditor.lastBlurPointY     = event.localY;
                                } else if (event.touchType === TouchType.Move) {
                                    imageContainer.showHelper(event.localX, event.localY);

                                    retouchEditor.changeImageAt(false, event.localX, event.localY);

                                    retouchEditor.lastBlurPointX = event.localX;
                                    retouchEditor.lastBlurPointY = event.localY;
                                } else {
                                    helperImageView.visible          = false;
                                    retouchEditor.lastBlurPointValid = false;
                                }
                            }
                        }

                        layoutProperties: AbsoluteLayoutProperties {
                            positionX: 0
                            positionY: 0
                        }

                        attachedObjects: [
                            RetouchEditor {
                                id:         retouchEditor
                                mode:       RetouchEditor.ModeScroll
                                scale:      imageScrollView.contentScale
                                helper:     helperImageView
                                helperSize: ui.sdu(20)
                                
                                onImageOpened: {
                                    activityIndicator.stop();
                                    activityIndicator.visible = false;
                                    
                                    saveActionItem.enabled = true;
                                    
                                    modeSegmentedControl.selectedOption = scrollModeOption;
                                    
                                    samplingPointModeOption.enabled = true;
                                    cloneModeOption.enabled         = true;
                                    blurModeOption.enabled          = true;

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
                                    blurModeOption.enabled          = false;

                                    samplingPointValid             = false;
                                    samplingPointImageView.visible = false;

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
                            },
                            LayoutUpdateHandler {
                                id: fragmentsContainerLayoutUpdateHandler
                            }
                        ]
                    }

                    ImageView {
                        id:                   samplingPointImageView
                        preferredWidth:       ui.sdu(6) / imageScrollView.contentScale
                        preferredHeight:      ui.sdu(6) / imageScrollView.contentScale
                        minWidth:             preferredWidth
                        minHeight:            preferredHeight
                        maxWidth:             preferredWidth
                        maxHeight:            preferredHeight
                        imageSource:          "images/sampling_point.png"
                        touchPropagationMode: TouchPropagationMode.PassThrough
                        overlapTouchPolicy:   OverlapTouchPolicy.Allow
                        visible:              false
                        accessibility.name:   qsTr("Sampling point")

                        layoutProperties: AbsoluteLayoutProperties {
                            positionX: 0
                            positionY: 0
                        }
                    }
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
