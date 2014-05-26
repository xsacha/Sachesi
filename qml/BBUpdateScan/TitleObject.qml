// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Rectangle {
    id: root
    signal clicked();
    property int obj: 0
    property int curObj: parent.curObj
    property alias text: textObj.text
    scale: parent.curObj === obj ? 1.0 : 0.8
    SystemPalette{id:pal}
    Behavior on scale { NumberAnimation { duration: 100 } }
    // Extend boundaries
    width: textObj.width + 10
    height: textObj.height + 20
    color: pal.mid
    radius: 8
    border.width: 2
    border.color: pal.shadow
    smooth: true
    Text {
        id: textObj
        font.pixelSize: 21
        anchors.horizontalCenter: parent.horizontalCenter
        color: root.curObj === root.obj ? pal.windowText : "#333333"
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
