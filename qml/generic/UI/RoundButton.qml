import QtQuick 2.2
import QtQuick.Controls 1.2

Button {
    id: root
    height: config.defaultButtonTextSize * 1.5
    width: Math.ceil(text.length / 3.0) * (config.defaultButtonTextSize * 2)
    property alias subtext: subtextValue.text
    Text {
        id: subtextValue
        anchors {left: parent.left; leftMargin: 0; top: parent.top; topMargin: parent.height}
        text: ""
        font.pixelSize: config.defaultSubtextSize; font.bold: true; color: "#404040"
    }
}
