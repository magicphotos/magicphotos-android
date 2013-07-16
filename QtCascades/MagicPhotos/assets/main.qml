import bb.cascades 1.0
import bb.system 1.0
import FilePicker 1.0
import CustomTimer 1.0

NavigationPane {
    id:          navigationPane
    peekEnabled: false

    onCreationCompleted: {
        OrientationSupport.supportedDisplayOrientation = SupportedDisplayOrientation.All;

        AppSettings.launchNumber = AppSettings.launchNumber + 1;
    }

    onPopTransitionEnded: {
        page.destroy();
    }

    Page {
        id: modeSelectionPage
        
        onCreationCompleted: {
            modeChangeSuggestionTimer.start();
        }
        
        actions: [
            ActionItem {
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
            background: Color.Black

            layout: StackLayout {
            }

            ListView {
                id:            modeSelectionListView
                snapMode:      SnapMode.LeadingEdge
                flickMode:     FlickMode.SingleItem
                rootIndexPath: [0]

                property int actualWidth:  0
                property int actualHeight: 0

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

                            property string itemMode:  ListItemData.mode
                            property string itemImage: ListItemData.image 

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
                                imageSource:         itemImage
                                scalingMethod:       ScalingMethod.AspectFit

                                layoutProperties: StackLayoutProperties {
                                    spaceQuota: 1
                                }
                            }
                            
                            Button {
                                id:                  modeButton
                                horizontalAlignment: HorizontalAlignment.Center
                                text:                qsTr("Open Image")
                                
                                onClicked: {
                                    openFilePicker.open();
                                }
                                
                                layoutProperties: StackLayoutProperties {
                                    spaceQuota: -1
                                }

                                attachedObjects: [
                                    FilePicker {
                                        id:    openFilePicker
                                        type:  FileType.Picture
                                        mode:  FilePickerMode.Picker
                                        title: qsTr("Open Image")

                                        onFileSelected: {
                                            itemRoot.ListItem.view.navigateToEditPage(itemMode, selectedFiles[0]);
                                        } 
                                    }
                                ]
                            }
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
