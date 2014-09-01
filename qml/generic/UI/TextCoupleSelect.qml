import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

Item {
    id: comboBox
    property string type
    property string initialText
    property alias text: comboButton.currentText
    property alias subtext: subtextValue.text
    property alias listModel: comboButton.model
    property alias selectedItem: comboButton.currentIndex

    // Evil: hardcoded width/height
    height: (14.5) * 1.4
    width: (14.5) * 14
    ColumnLayout {
        RowLayout {
            Layout.fillWidth: true
            Label {
                id: typeText
                text: type
                font.bold: true
            }
            ComboBox {
                id: comboButton
                currentIndex: selectedItem
                anchors { left: parent.left; leftMargin: comboBox.width / 2 - 10}
            }
        }
        Label {
            id: subtextValue
            anchors {left: parent.left; leftMargin: 10}
            text: ""
        }
    }
}
