import QtQuick 1.1

Item {
    id: itemroot
    property string type
    property alias value: textValue.text
    property alias subtext: subtextValue.text
    property alias thisid: textValue
    property alias readOnly: textValue.readOnly
    property alias textColor: textValue.color
    property variant before: itemroot
    property variant after: itemroot
    height: 20; width: 200
    Text {
        anchors {left: parent.left; leftMargin: 10}
        text: type
        font.pixelSize: 15; font.bold: true
    }
    Text {
        id: subtextValue
        anchors {left: parent.left; leftMargin: 15; top: parent.top; topMargin: 18}
        text: ""
        font.pixelSize: 10; font.bold: true; color: "#404040"
    }
    Rectangle {
        anchors {left: parent.left; leftMargin: 100}
        height: 20; width: 90
        radius: 2
        TextEdit {
            id: textValue
            width: parent.width
            anchors {left: parent.left; leftMargin: 5; verticalCenter: parent.verticalCenter }
            font.pixelSize: 11
            text: value
            KeyNavigation.priority: KeyNavigation.BeforeItem
            KeyNavigation.tab: after
            KeyNavigation.backtab: before
            Keys.onReturnPressed: {}
            Keys.onEnterPressed: {}
        }
    }
}
