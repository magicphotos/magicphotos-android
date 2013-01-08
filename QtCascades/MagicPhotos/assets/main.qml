import bb.cascades 1.0
import FilePicker 1.0

NavigationPane {
    id:          navigationPane
    peekEnabled: false

    onCreationCompleted: {
        OrientationSupport.supportedDisplayOrientation = SupportedDisplayOrientation.All;
    }

    onPopTransitionEnded: {
        page.destroy();
    }

    Page {
        id: modeSelectionPage
        
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

                            layout: StackLayout {
                            }

                            Label {
                                id:                      modeLabel
                                horizontalAlignment:     HorizontalAlignment.Left
                                text:                    qsTr(ListItemData.name)
                                textStyle.fontSize:      FontSize.PercentageValue
                                textStyle.fontSizeValue: 250
                                textStyle.color:         Color.White

                                layoutProperties: StackLayoutProperties {
                                    spaceQuota: -1
                                }
                            }

                            ImageView {
                                id:                  modeImageView
                                preferredWidth:      itemRoot.ListItem.view.actualWidth
                                horizontalAlignment: HorizontalAlignment.Center
                                imageSource:         ListItemData.image
                                scalingMethod:       ScalingMethod.AspectFit

                                layoutProperties: StackLayoutProperties {
                                    spaceQuota: 1
                                }
                            }
                            
                            Button {
                                id:                  modeButton
                                horizontalAlignment: HorizontalAlignment.Center
                                text:                qsTr("Open Image")
                                
                                layoutProperties: StackLayoutProperties {
                                    spaceQuota: -1
                                }

                                onClicked: {
                                    openFilePicker.open();
                                }
                                
                                attachedObjects: [
                                    FilePicker {
                                        id:    openFilePicker
                                        type:  FileType.Picture
                                        mode:  FilePickerMode.Picker
                                        title: qsTr("Open Image")

                                        onFileSelected: {
                                            itemRoot.ListItem.view.navigateToEditPage(ListItemData.mode, selectedFiles[0]);
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
        }
    }
}
