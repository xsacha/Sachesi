// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.0

Rectangle {
    id: root
    signal clicked();
    property int obj: 0
    property int curObj: parent.curObj
    property alias text: textObj.text
    scale: parent.curObj === obj ? 1.0 : 0.8
    Behavior on scale { NumberAnimation { duration: 100 } }
    // Extend boundaries
    width: textObj.width + 10
    height: textObj.height
    color: config.darkColor
    border.width: 2
    border.color: config.shadowColor
    smooth: true
    Text {
        id: textObj
        font.pixelSize: config.notificationFontSize
        y: -1
        anchors.horizontalCenter: parent.horizontalCenter
        color: root.curObj === root.obj ? "black" : "#333333"
        Behavior on color { ColorAnimation { duration: 100 } }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                root.parent.curObj = root.obj
                root.clicked();
            }
        }
    }
}
