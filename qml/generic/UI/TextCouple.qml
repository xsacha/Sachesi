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
    property alias restrictions: textValue.inputMethodHints
    property alias maxLength: textValue.maximumLength
    property string helpLink
    signal clicked();
    // Evil: hardcoded width/height
    height: (14.5) * 1.5
    width: (14.5) * 15
    ColumnLayout {
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: type
                font.bold: true
                Rectangle {
                    visible: helpLink.length > 0
                    color: "#AAAAAA"
                    anchors { left: parent.right; leftMargin: 5 }
                    width: childrenRect.width
                    height: childrenRect.height
                    radius: 4
                    Label {
                        text: "?"
                        font.bold: true
                        font.pointSize: 14
                        MouseArea {
                            anchors.fill: parent
                            onClicked: Qt.openUrlExternally(helpLink);
                        }
                    }
                }
            }
            TextField {
                id: textValue
                anchors {left: parent.left; leftMargin: itemroot.width / 2 - 10}
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
