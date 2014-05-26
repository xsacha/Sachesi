// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Rectangle {
    id: root
    width: textObj.width + 10 + textObj.font.pointSize
    height: textObj.height + 10
    color: "transparent"
    property alias text: textObj.text
    property bool enabled: true
    property bool checked: false
    property bool setOnDisable: true
    signal selected();
    Rectangle {
        color: "white"
        radius: 2
        width: textObj.font.pointSize + 2; height: textObj.font.pointSize + 2
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        Text {
            anchors {
                horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter
                verticalCenterOffset: -1
            }
            text: (root.checked || (!root.enabled && setOnDisable)) ? "x" : ""
            color: root.enabled ? "black" : "grey"
            font.pointSize: textObj.font.pointSize - 2
        }
        MouseArea {
            enabled: root.enabled
            anchors.fill: parent
            onClicked: {root.checked = !root.checked; root.selected(); }
        }
    }

    Text {
        id: textObj
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        font.pointSize: 12
    }
}
