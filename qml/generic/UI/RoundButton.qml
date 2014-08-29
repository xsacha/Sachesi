import QtQuick 2.2
import QtQuick.Controls 1.2

Button {
    id: root
    property alias subtext: subtextValue.text
    Text {
        id: subtextValue
        anchors {left: parent.left; leftMargin: 0; top: parent.top; topMargin: parent.height}
        text: ""
        font.bold: true;
    }
}
