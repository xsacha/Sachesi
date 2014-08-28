import QtQuick 2.2

Rectangle {
    id: root
    property int size;
    color: "transparent"
    width: size; height: size
    Image {
        source: "../circle.png";
        id: circle
        SequentialAnimation {
            running: true
            NumberAnimation { target: circle; property: "rotation"; alwaysRunToEnd: true; duration: 1000; from: 0; to: 360; }
            onRunningChanged: running = true;
        }
        anchors.fill: parent
    }
}
