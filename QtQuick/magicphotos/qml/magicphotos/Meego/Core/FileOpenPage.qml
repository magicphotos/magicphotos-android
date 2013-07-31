import QtQuick 1.1
import QtMobility.gallery 1.1
import com.nokia.meego 1.0

Page {
    id:           fileOpenPage
    anchors.fill: parent

    property variant caller: null

    signal fileSelected(string file_url)

    onStatusChanged: {
        if (status === PageStatus.Active) {
            fileSelected.connect(caller.fileSelected);

            imageGridView.currentIndex = -1;

            documentGalleryModel.reload();
        }
    }

    function utf8Decode(string) {
        var encoded_string = unescape(string);
        var decoded_string = "";
        var i              = 0;
        var c, c1, c2;

        while (i < encoded_string.length) {
            c = encoded_string.charCodeAt(i);

            if (c < 128) {
                decoded_string = decoded_string + String.fromCharCode(c);
                i              = i + 1;
            } else if ((c > 191) && (c < 224)) {
                c1             = encoded_string.charCodeAt(i + 1);
                decoded_string = decoded_string + String.fromCharCode(((c & 31) << 6) | (c1 & 63));
                i              = i + 2;
            } else {
                c1             = encoded_string.charCodeAt(i + 1);
                c2             = encoded_string.charCodeAt(i + 2);
                decoded_string = decoded_string + String.fromCharCode(((c & 15) << 12) | ((c1 & 63) << 6) | (c2 & 63));
                i              = i + 3;
            }
        }

        return decoded_string;
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
                        anchors.fill: parent

                        onClicked: {
                            imageGridView.currentIndex = index;
                        }

                        Image {
                            anchors.centerIn: parent
                            width:            parent.width  - galleryItemRectangle.border.width
                            height:           parent.height - galleryItemRectangle.border.width
                            source:           fileOpenPage.utf8Decode(url)
                            sourceSize.width: width
                            cache:            false
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
                anchors.fill: parent

                Image {
                    anchors.centerIn: parent
                    source:           "../../images/busy_indicator.png"
                }
            }
        }
    }

    ToolBar {
        id:             bottomToolBar
        anchors.bottom: parent.bottom
        z:              1

        tools: ToolBarLayout {
            ToolButton {
                iconSource: "../images/back.png"
                flat:       true

                onClicked: {
                    mainPageStack.pop();
                }
            }

            ToolButton {
                iconSource: "../images/ok.png"
                enabled:    imageGridView.currentIndex !== -1
                flat:       true

                onClicked: {
                    if (imageGridView.currentIndex !== -1) {
                        fileOpenPage.fileSelected(fileOpenPage.normalizeFileUrl(fileOpenPage.utf8Decode(documentGalleryModel.property(imageGridView.currentIndex, "url"))));
                    }
                }
            }
        }
    }
}
