import QtQuick 1.0
import QtMobility.gallery 1.1
import com.nokia.symbian 1.0

Page {
    id:           fileOpenPage
    anchors.fill: parent

    signal fileSelected(string fileUrl)
    signal openCancelled()

    onStatusChanged: {
        if (status === PageStatus.Activating) {
            imageGridView.visible = false;
            waitRectangle.visible = true;
        } else if (status === PageStatus.Active) {
            imageGridView.currentIndex = -1;

            documentGalleryModel.reload();
        }
    }

    function normalizeFileUrl(file_url) {
        var fileUrlRegexp = /^file:\/\/\/.+$/;

        if (fileUrlRegexp.exec(file_url) !== null) {
            return file_url;
        } else {
            return "file:///" + file_url;
        }
    }

    Rectangle {
        id:             gridViewBackground
        anchors.top:    parent.top
        anchors.bottom: bottomToolBar.top
        anchors.left:   parent.left
        anchors.right:  parent.right
        color:          "black"

        GridView {
            id:           imageGridView
            anchors.fill: parent
            cellWidth:    width  > height ? Math.floor(width  / 5) : Math.floor(width  / 3)
            cellHeight:   height > width  ? Math.floor(height / 5) : Math.floor(height / 3)
            model:        documentGalleryModel
            delegate:     documentGalleryDelegate
            visible:      false

            DocumentGalleryModel {
                id:             documentGalleryModel
                rootType:       DocumentGallery.Image
                autoUpdate:     true
                properties:     ["url"]
                sortProperties: ["-lastModified"]

                onStatusChanged: {
                    if (status == DocumentGalleryModel.Finished || status === DocumentGalleryModel.Idle) {
                        waitRectangle.visible = false;
                        imageGridView.visible = true;
                    }
                }
            }

            Component {
                id: documentGalleryDelegate

                Rectangle {
                    id:           galleryItemRectangle
                    width:        imageGridView.cellWidth  - border.width
                    height:       imageGridView.cellHeight - border.width
                    color:        "transparent"
                    border.color: GridView.isCurrentItem ? "white" : "steelblue"
                    border.width: 2

                    MouseArea {
                        id:           galleryItemMouseArea
                        anchors.fill: parent

                        onClicked: {
                            imageGridView.currentIndex = index;
                        }

                        Image {
                            id:               galleryItemImage
                            anchors.centerIn: parent
                            width:            parent.width  - galleryItemRectangle.border.width
                            height:           parent.height - galleryItemRectangle.border.width
                            source:           url
                            sourceSize.width: width
                            asynchronous:     true
                            fillMode:         Image.PreserveAspectFit
                            smooth:           false
                        }
                    }
                }
            }
        }

        Rectangle {
            id:           waitRectangle
            anchors.fill: parent
            color:        "transparent"

            MouseArea {
                id:           waitRectangleMouseArea
                anchors.fill: parent

                Image {
                    id:                       waitBusyIndicatorImage
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter:   parent.verticalCenter
                    source:                   "qrc:/resources/images/busy_indicator.png"

                    NumberAnimation on rotation {
                        running: waitRectangle.visible
                        from:    0
                        to:      360
                        loops:   Animation.Infinite
                    }
                }
            }
        }
    }

    ToolBar {
        id:             bottomToolBar
        anchors.bottom: parent.bottom
        z:              1

        tools: ToolBarLayout {
            ButtonRow {
                id:        bottomToolBarButtonRow
                exclusive: false

                ToolButton {
                    id:         openToolButton
                    iconSource: "qrc:/resources/images/open.png"
                    flat:       true
                    enabled:    imageGridView.currentIndex !== -1

                    onClicked: {
                        if (imageGridView.currentIndex !== -1) {
                            fileOpenPage.fileSelected(fileOpenPage.normalizeFileUrl(documentGalleryModel.property(imageGridView.currentIndex, "url")));
                        }
                    }
                }

                ToolButton {
                    id:         openCancelToolButton
                    iconSource: "qrc:/resources/images/cancel.png"
                    flat:       true

                    onClicked: {
                        fileOpenPage.openCancelled();
                    }
                }
            }
        }
    }
}
