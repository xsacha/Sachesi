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

    height: config.defaultButtonTextSize * 1.4; width: config.defaultButtonTextSize * 14
    Label {
        anchors {left: parent.left; leftMargin: 10}
        text: type
        font.bold: true
    }
    Label {
        id: subtextValue
        anchors {left: parent.left; leftMargin: 15; top: parent.top; topMargin: parent.height + 5}
        text: ""
    }
    ComboBox {
        id: comboButton
        currentIndex: selectedItem
        anchors { left: parent.left; leftMargin: comboBox.width / 2 - 10}
    }
}
