import QtQuick 2.2
import QtQuick.Controls 1.2

Item {
    id: comboBox
    property string type
    property string initialText
    property alias text: comboButton.currentText
    property alias subtext: subtextValue.text
    property alias listModel: comboButton.model
    property alias selectedItem: comboButton.currentIndex
    SystemPalette {id: pal}

    height: config.defaultButtonTextSize * 1.4; width: config.defaultButtonTextSize * 14
    Text {
        anchors {left: parent.left; leftMargin: 10}
        text: type
        font.pixelSize: config.defaultButtonTextSize; font.bold: true
    }
    Text {
        id: subtextValue
        anchors {left: parent.left; leftMargin: 15; top: parent.top; topMargin: parent.height - 2}
        text: ""
        font.pixelSize: config.defaultSubtextSize; font.bold: true; color: "#404040"
    }
    ComboBox {
        id: comboButton
        currentIndex: selectedItem
        anchors {left: parent.left; leftMargin: comboBox.width / 2 - 10}
        height: comboBox.height; width: (config.defaultSubtextSize + 1) * 9
        model: [ initialText ]
    }
/*    Rectangle {
        id: comboButton
        property string text: initialText
        anchors {left: parent.left; leftMargin: comboBox.width / 2 - 10}
        height: comboBox.height; width: (config.defaultSubtextSize + 1) * 7
        radius: 2
        Text {
            anchors {left: parent.left; leftMargin: 5; verticalCenter: parent.verticalCenter }
            font.pixelSize: config.defaultSubtextSize + 1
            text: parent.text
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (listView.height == 0) {
                    listView.height = listModel.count*comboButton.height
                    comboBox.expanded()
                }
                else {
                    listView.height = 0
                    comboBox.closed()
                }
            }
        }
    }
    Component {
        id: comboBoxDelegate
        Rectangle {
            id: delegateRectangle
            x: 2; y: 2
            width: comboButton.width - 4
            height: comboButton.height
            color: pal.window

            Item {
                anchors.left: parent.left
                width: parent.width-parent.height
                anchors.top: parent.top
                anchors.bottom: parent.bottom

                Text {
                    anchors.left: parent.left; anchors.leftMargin: 2
                    font.pixelSize: config.defaultSubtextSize
                    font.bold: index === selectedItem
                    text: modelData
                }
            }

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    listView.height = 0
                    listView.currentIndex = index
                    comboBox.selectedItem = index
                    comboButton.text = name
                }
            }
        }
    }
    ListView {
        id: listView
        height: 0
        anchors.top: comboButton.bottom
        anchors.left: comboButton.left
        width: comboButton.width
        clip: true
        model: listModel
        delegate: comboBoxDelegate
        currentIndex: selectedItem

        Behavior on height { NumberAnimation { duration: 100 } }
    }
*/
}
