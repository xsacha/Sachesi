import QtQuick 1.1

Item {
    id: root
    height: 20
    width: Math.ceil(text.length / 3.0) * 24
    property alias mouse: mouseButton
    property alias text: textButton.text
    property alias subtext: subtextValue.text
    property bool enabled: true
    SystemPalette {id:pal}

    Rectangle {
        anchors.fill: parent

        border.color: "#666666"
        border.width: 1
        scale: mouseButton.pressed ? 0.9 : 1.0
        Behavior on scale { NumberAnimation { duration: 100 } }
        radius: mouseButton.pressed ? 4 : 6
        Behavior on radius { NumberAnimation { duration: 100 } }

        smooth: true
        gradient: Gradient {
            GradientStop { position: 0.0; color: root.enabled ? pal.button : pal.dark }
            GradientStop { position: 1.0; color: pal.dark }
        }
        Text {
            id: textButton
            font.bold: true
            font.pixelSize: 12
            anchors.centerIn: parent
        }
        MouseArea {
            id: mouseButton
            enabled: root.enabled
            anchors.fill: parent
        }
    }
    Text {
        id: subtextValue
        anchors {left: parent.left; leftMargin: 0; top: parent.top; topMargin: 20}
        text: ""
        font.pixelSize: 10; font.bold: true; color: "#404040"
    }
}
