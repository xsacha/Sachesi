import QtQuick 1.1

Item {
    id: comboBox
    property string type
    property string initialText
    property alias text: comboButton.text
    property variant listModel
    property int selectedItem: 0
    signal expanded()
    signal closed()
    function close() {
        listView.height = 0
        this.closed()
    }
    SystemPalette {id: pal}

    height: 20; width: 200
    Text {
        anchors {left: parent.left; leftMargin: 10}
        text: type
        font.pixelSize: 15; font.bold: true
    }
    Rectangle {
        id: comboButton
        property string text: initialText
        anchors {left: parent.left; leftMargin: 100}
        height: 20; width: 90
        radius: 2
        Text {
            anchors {left: parent.left; leftMargin: 5; verticalCenter: parent.verticalCenter }
            font.pixelSize: 11
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
                    font.pixelSize: 10
                    font.bold: index === listView.currentIndex
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
}
