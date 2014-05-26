// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Rectangle {
    id: main
    width: 480
    height: 340
    radius: 8
    SystemPalette {id: pal}
    border.width: 2
    border.color: pal.shadow
    gradient: Gradient {
        GradientStop { position: 0.0; color: pal.mid }
        GradientStop { position: 1.0; color: pal.midlight }
    }
    Column {
        anchors {left: parent.left; leftMargin: 20; top: parent.top; topMargin: 20 }
        spacing: 20
        Text {
            text: "1. Appworld exploit.\n Requires OS: Any"
            font.pointSize: 12
        }
        Text {
            text: "2. Root exploit.\n Requires OS: 10.0.0 - 10.0.9"
            font.pointSize: 12
        }
    }
    Text {
        text: "Coming Soon"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 80
        font.pixelSize: 30
    }
}
