import QtQuick 1.1
import com.nokia.symbian 1.1

Dialog {
    id: saveDialog

    property string fileUrlPath: ""

    signal done(string file_url_path, string file_name)

    function show(file_url) {
        if (file_url !== "") {
            var urlPathNameRegexp = /^(.+)\/([^/]+)$/;
            var urlPathNameArr;

            if ((urlPathNameArr = urlPathNameRegexp.exec(file_url)) !== null) {
                fileUrlPath            = urlPathNameArr[1];
                fileNameTextField.text = urlPathNameArr[2];
            } else {
                fileUrlPath            = "";
                fileNameTextField.text = "";
            }
        }

        open();
    }

    title: [
        Text {
            id:                     saveDialogTitleText
            text:                   "File Name"
            anchors.verticalCenter: parent.verticalCenter
            x:                      10
            color:                  "steelblue"
        },
        Image {
            anchors.verticalCenter: parent.verticalCenter
            x:                      parent.width - width - 10
            source:                 "../../../images/dialog_question.png"
        }
    ]

    content: [
        TextField {
            id:                     fileNameTextField
            anchors.verticalCenter: parent.verticalCenter
            anchors.left:           parent.left
            anchors.right:          parent.right
        }
    ]

    buttons: [
        Button {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left:           parent.left
            width:                  parent.width / 2 - 4
            text:                   "OK"

            onClicked: {
                saveDialog.done(saveDialog.fileUrlPath, fileNameTextField.text);

                saveDialog.accept();
            }
        },
        Button {
            anchors.verticalCenter: parent.verticalCenter
            anchors.right:          parent.right
            width:                  parent.width / 2 - 4
            text:                   "Cancel"

            onClicked: {
                saveDialog.reject();
            }
        }
    ]
}
