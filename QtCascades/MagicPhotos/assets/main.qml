import bb.cascades 1.3
import bb.system 1.2
import FilePicker 1.0
import CustomTimer 1.0
import ImageEditor 1.0

NavigationPane {
    id:          navigationPane
    peekEnabled: false

    onCreationCompleted: {
        OrientationSupport.supportedDisplayOrientation = SupportedDisplayOrientation.All;

        AppSettings.launchNumber     = AppSettings.launchNumber + 1;
        AppSettings.defaultBrushSize = ui.sdu(3);
    }

    onPopTransitionEnded: {
        page.destroy();
    }

    Menu.definition: MenuDefinition {
        settingsAction: SettingsActionItem {
            onTriggered: {
                var resolution_limit = AppSettings.imageResolutionLimit;

                brushSizeSlider.value    = AppSettings.brushSize;
                brushOpacitySlider.value = AppSettings.brushOpacity;

                if (resolution_limit > 1.9) {
                    imageResolutionLimitDropDown.selectedIndex = 2;
                } else if (resolution_limit > 0.9) {
                    imageResolutionLimitDropDown.selectedIndex = 1;
                } else if (resolution_limit > 0.4) {
                    imageResolutionLimitDropDown.selectedIndex = 0;
                } else {
                    imageResolutionLimitDropDown.selectedIndex = 3;
                }

                settingsSheet.open();
            }
            
            attachedObjects: [
                Sheet {
                    id: settingsSheet
                    
                    Page {
                        id: settingsPage
                        
                        titleBar: TitleBar {
                            title: qsTr("Settings")
                            
                            acceptAction: ActionItem {
                                title: qsTr("OK")
                                
                                onTriggered: {
                                    var resolution_limit = AppSettings.imageResolutionLimit;

                                    if (imageResolutionLimitDropDown.selectedIndex === 0) {
                                        resolution_limit = 0.5;
                                    } else if (imageResolutionLimitDropDown.selectedIndex === 1) {
                                        resolution_limit = 1.0;
                                    } else if (imageResolutionLimitDropDown.selectedIndex === 2) {
                                        resolution_limit = 2.0;
                                    } else if (imageResolutionLimitDropDown.selectedIndex === 3) {
                                        resolution_limit = 0.0;
                                    }

                                    AppSettings.brushSize            = brushSizeSlider.value;
                                    AppSettings.brushOpacity         = brushOpacitySlider.value;
                                    AppSettings.imageResolutionLimit = resolution_limit;
                                    
                                    for (var i = 0; i < navigationPane.count(); i++) {
                                        var page = navigationPane.at(i);
                                        
                                        if (page.objectName === "editorPage") {
                                            page.updateEditorParameters();
                                        }
                                    }
                                    
                                    settingsSheet.close();
                                }
                            }
                            
                            dismissAction: ActionItem {
                                title: qsTr("Cancel")
                                
                                onTriggered: {
                                    settingsSheet.close();
                                }
                            }
                        }
                        
                        Container {
                            background: Color.Black
                            
                            ScrollView {
                                accessibility.name: qsTr("Settings")
                                
                                scrollViewProperties {
                                    scrollMode: ScrollMode.Vertical
                                }
                                
                                Container {
                                    background: Color.Transparent
                                    
                                    leftPadding:  ui.sdu(1)
                                    rightPadding: ui.sdu(1)

                                    Divider {
                                        accessibility.name: qsTr("Divider")
                                    }

                                    Container {
                                        background: Color.Transparent
                                        
                                        layout: StackLayout {
                                            orientation: LayoutOrientation.LeftToRight
                                        }
                                        
                                        Container {
                                            background: Color.Transparent
                                            
                                            layout: StackLayout {
                                            }
                                            
                                            layoutProperties: StackLayoutProperties {
                                                spaceQuota: 3
                                            }
                                            
                                            Label {
                                                multiline:       true
                                                textStyle.color: Color.White
                                                text:            qsTr("Brush Size")
                                            }
                                            
                                            Slider {
                                                id:                  brushSizeSlider
                                                horizontalAlignment: HorizontalAlignment.Fill
                                                fromValue:           ui.sdu(2)
                                                toValue:             ui.sdu(4)
                                                value:               ui.sdu(3)
                                                accessibility.name:  qsTr("Brush size slider")
                                            }
                                            
                                            Label {
                                                multiline:       true
                                                textStyle.color: Color.White
                                                text:            qsTr("Brush Opacity")
                                            }
                                            
                                            Slider {
                                                id:                  brushOpacitySlider
                                                horizontalAlignment: HorizontalAlignment.Fill
                                                fromValue:           0.0
                                                toValue:             1.0
                                                value:               0.75
                                                accessibility.name:  qsTr("Brush opacity slider")
                                            }
                                        }
                                        
                                        ImageView {
                                            id:                  brushPreviewImageView
                                            horizontalAlignment: HorizontalAlignment.Center
                                            verticalAlignment:   VerticalAlignment.Center
                                            scalingMethod:       ScalingMethod.None
                                            accessibility.name:  qsTr("Brush preview")
                                            
                                            layoutProperties: StackLayoutProperties {
                                                spaceQuota: 1
                                            }
                                            
                                            attachedObjects: [
                                                BrushPreviewGenerator {
                                                    id:      brushPreviewGenerator
                                                    size:    brushSizeSlider.immediateValue
                                                    maxSize: brushSizeSlider.toValue
                                                    opacity: brushOpacitySlider.immediateValue
                                                    preview: brushPreviewImageView
                                                }
                                            ]
                                        }
                                    }

                                    Divider {
                                        accessibility.name: qsTr("Divider")
                                    }

                                    DropDown {
                                        id:    imageResolutionLimitDropDown
                                        title: qsTr("Image Quality")

                                        Option {
                                            text: qsTr("Low (<= 0.5 Mpix)")
                                        }

                                        Option {
                                            text: qsTr("Medium (<= 1.0 Mpix)")
                                        }

                                        Option {
                                            text: qsTr("High (<= 2.0 Mpix)")
                                        }

                                        Option {
                                            text: qsTr("Original")
                                        }
                                    }

                                    Divider {
                                        accessibility.name: qsTr("Divider")
                                    }
                                }
                            }
                        }
                    }
                }
            ]
        }
        
        helpAction: HelpActionItem {
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
    }

    Page {
        id: modeSelectionPage
        
        onCreationCompleted: {
            modeChangeSuggestionTimer.start();
        }
        
        actions: [
            ActionItem {
                title:               qsTr("Open")
                imageSource:         "images/open.png"
                ActionBar.placement: ActionBarPlacement.Signature
                
                onTriggered: {
                    modeSelectionListView.openImageInSelectedMode();
                }
            }
        ]
        
        Container {
            background: Color.Black

            layout: StackLayout {
            }

            ListView {
                id:                 modeSelectionListView
                snapMode:           SnapMode.LeadingEdge
                flickMode:          FlickMode.SingleItem
                rootIndexPath:      [0]
                accessibility.name: qsTr("List of modes")

                property int actualWidth:  0
                property int actualHeight: 0

                property variant invocationHelper: InvocationHelper

                signal openImageInSelectedMode()

                function navigateToEditPage(mode, image_file) {
                    if (mode === "DECOLORIZE") {
                        editPageDefinition.source = "DecolorizePage.qml";

                        var page = editPageDefinition.createObject();

                        navigationPane.push(page);

                        page.openImage(image_file);
                    } else if (mode === "SKETCH") {
                        editPageDefinition.source = "SketchPreviewPage.qml";

                        var page = editPageDefinition.createObject();

                        navigationPane.push(page);

                        page.openImage(image_file);
                    } else if (mode === "CARTOON") {
                        editPageDefinition.source = "CartoonPreviewPage.qml";

                        var page = editPageDefinition.createObject();

                        navigationPane.push(page);

                        page.openImage(image_file);
                    } else if (mode === "BLUR") {
                        editPageDefinition.source = "BlurPreviewPage.qml";

                        var page = editPageDefinition.createObject();

                        navigationPane.push(page);

                        page.openImage(image_file);
                    } else if (mode === "PIXELATE") {
                        editPageDefinition.source = "PixelatePreviewPage.qml";

                        var page = editPageDefinition.createObject();

                        navigationPane.push(page);

                        page.openImage(image_file);
                    } else if (mode === "RECOLOR") {
                        editPageDefinition.source = "RecolorPage.qml";

                        var page = editPageDefinition.createObject();

                        navigationPane.push(page);

                        page.openImage(image_file);
                    } else if (mode === "RETOUCH") {
                        editPageDefinition.source = "RetouchPage.qml";

                        var page = editPageDefinition.createObject();

                        navigationPane.push(page);

                        page.openImage(image_file);
                    }
                }

                layout: StackListLayout {
                    orientation: LayoutOrientation.LeftToRight                            
                }

                dataModel: XmlDataModel {
                    source: "models/modeSelectionListViewModel.xml"                               
                }

                listItemComponents: [
                    ListItemComponent {
                        type: "item"

                        Container {
                            id:              itemRoot
                            preferredWidth:  ListItem.view.actualWidth
                            preferredHeight: ListItem.view.actualHeight
                            background:      Color.Transparent

                            property bool itemSelected: false

                            property string itemMode:  ListItemData.mode
                            property string itemImage: ListItemData.image 

                            signal openImageInSelectedMode()

                            onCreationCompleted: {
                                ListItem.view.openImageInSelectedMode.connect(openImageInSelectedMode);
                            }

                            onItemModeChanged: {
                                if (itemMode === "DECOLORIZE") {
                                    modeLabel.text = qsTr("Decolorize");
                                } else if (itemMode === "SKETCH") {
                                    modeLabel.text = qsTr("Sketch");
                                } else if (itemMode === "CARTOON") {
                                    modeLabel.text = qsTr("Cartoon");
                                } else if (itemMode === "BLUR") {
                                    modeLabel.text = qsTr("Blur");
                                } else if (itemMode === "PIXELATE") {
                                    modeLabel.text = qsTr("Pixelate");
                                } else if (itemMode === "RECOLOR") {
                                    modeLabel.text = qsTr("Recolor");
                                } else if (itemMode === "RETOUCH") {
                                    modeLabel.text = qsTr("Retouch");
                                }
                            }

                            onOpenImageInSelectedMode: {
                                if (itemSelected) {
                                    if (ListItem.view.invocationHelper.imageFile !== "") {
                                        var image_file = ListItem.view.invocationHelper.imageFile;
                                        
                                        ListItem.view.invocationHelper.imageFile = "";
                                        
                                        ListItem.view.navigateToEditPage(itemMode, image_file);
                                    } else {
                                        openFilePicker.open();
                                    }
                                }
                            }

                            layout: StackLayout {
                            }

                            Label {
                                id:                      modeLabel
                                horizontalAlignment:     HorizontalAlignment.Left
                                textStyle.fontSize:      FontSize.PercentageValue
                                textStyle.fontSizeValue: 200
                                textStyle.color:         Color.White

                                layoutProperties: StackLayoutProperties {
                                    spaceQuota: -1
                                }
                            }

                            ImageView {
                                id:                  modeImageView
                                preferredWidth:      itemRoot.ListItem.view.actualWidth
                                horizontalAlignment: HorizontalAlignment.Center
                                imageSource:         itemRoot.itemImage
                                scalingMethod:       ScalingMethod.AspectFit
                                accessibility.name:  qsTr("Example image")

                                layoutProperties: StackLayoutProperties {
                                    spaceQuota: 1
                                }
                                
                                gestureHandlers: [
                                    TapHandler {
                                        onTapped: {
                                            if (itemRoot.ListItem.view.invocationHelper.imageFile !== "") {
                                                var image_file = itemRoot.ListItem.view.invocationHelper.imageFile;

                                                itemRoot.ListItem.view.invocationHelper.imageFile = "";

                                                itemRoot.ListItem.view.navigateToEditPage(itemRoot.itemMode, image_file);
                                            } else {
                                                openFilePicker.open();
                                            }
                                        }
                                    }
                                ]
                            }

                            attachedObjects: [
                                LayoutUpdateHandler {
                                    onLayoutFrameChanged: {
                                        if (layoutFrame.x >= -itemRoot.ListItem.view.actualWidth / 2 &&
                                            layoutFrame.x <=  itemRoot.ListItem.view.actualWidth / 2) {
                                            itemRoot.itemSelected = true;
                                        } else {
                                            itemRoot.itemSelected = false;
                                        }
                                    }
                                },
                                FilePicker {
                                    id:    openFilePicker
                                    type:  FileType.Picture
                                    mode:  FilePickerMode.Picker
                                    title: qsTr("Open Image")

                                    onFileSelected: {
                                        itemRoot.ListItem.view.navigateToEditPage(itemRoot.itemMode, selectedFiles[0]);
                                    }
                                }
                            ]
                        } 
                    }
                ]

                attachedObjects: [
                    LayoutUpdateHandler {
                        onLayoutFrameChanged: {
                            modeSelectionListView.actualWidth  = layoutFrame.width;
                            modeSelectionListView.actualHeight = layoutFrame.height;
                        }
                    },
                    ComponentDefinition {
                        id: editPageDefinition
                    }
                ]
            }
            
            attachedObjects: [
                SystemToast {
                    id:   modeChangeSuggestionToast
                    body: qsTr("Slide your finger over the mode selection screen to switch between modes")
                    
                    onFinished: {
                        modeSelectionListView.scrollToPosition(ScrollPosition.Beginning, ScrollAnimation.Smooth);
                    }
                },
                CustomTimer {
                    id:         modeChangeSuggestionTimer
                    singleShot: true
                    interval:   1000
                    
                    onTimeout: {
                        if (AppSettings.showModeChangeSuggestion) {
                            modeChangeSuggestionToast.show();

                            modeSelectionListView.scrollToPosition(ScrollPosition.End, ScrollAnimation.Smooth);

                            AppSettings.showModeChangeSuggestion = false;
                        }
                    }
                }
            ]
        }
    }
}
