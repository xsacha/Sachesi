import QtQuick 2.2

Item {
    id: root
    height: config.defaultButtonTextSize * 1.5
    width: Math.ceil(text.length / 3.0) * (config.defaultButtonTextSize * 2)
    signal clicked;
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
        radius: mouseButton.pressed ? 2 : 4
        Behavior on radius { NumberAnimation { duration: 100 } }

		smooth: true
		Image {
            source: "../section_background_pattern_odd.png"
			fillMode: Image.Tile
			anchors.fill: parent
			opacity: 0.6
		}
        gradient: Gradient {
            GradientStop { position: 0.0; color: root.enabled ? pal.button : pal.dark }
            GradientStop { position: 1.0; color: pal.dark }
		}
        Text {
            id: textButton
            font.bold: true
            font.pixelSize: config.defaultButtonTextSize
            anchors.centerIn: parent
        }
        MouseArea {
            id: mouseButton
            onClicked: root.clicked();
            enabled: root.enabled
            anchors.fill: parent
        }
    }
    Text {
        id: subtextValue
        anchors {left: parent.left; leftMargin: 0; top: parent.top; topMargin: parent.height}
        text: ""
        font.pixelSize: config.defaultSubtextSize; font.bold: true; color: "#404040"
    }
}
