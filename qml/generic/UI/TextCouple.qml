import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

Item {
    id: itemroot
    property string type
    property alias value: textValue.text
    property alias subtext: subtextValue.text
    property alias thisid: textValue
    property alias textColor: textValue.textColor
    property alias typeOffset: typeText.right
    property alias restrictions: textValue.inputMethodHints
    property alias maxLength: textValue.maximumLength
    signal clicked();
    // Evil: hardcoded width/height
    height: (14.5) * 1.4
    width: (14.5) * 18
    ColumnLayout {
        RowLayout {
            Layout.fillWidth: true
            Label {
                id: typeText
                text: type
                font.bold: true
            }
            TextField {
                id: textValue
                anchors {left: parent.left; leftMargin: itemroot.width / 2 - 40}
                text: value
                onAccepted: itemroot.clicked()
            }
        }
        Label {
            id: subtextValue
            anchors {left: parent.left; leftMargin: 10}
            text: ""
        }
    }
}
