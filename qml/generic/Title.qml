import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Window 2.1
import Qt.labs.settings 1.0
import "UI" 1.0

ApplicationWindow {
    id: window
    title: "Sachesi " + version + " " + "Beta"
    width: 820
    height: 680
    minimumHeight: 540
    minimumWidth: 640

    Settings {
        id: settings
        property alias x: window.x
        property alias y: window.y
        property alias width: window.width
        property alias height: window.height
        property url installFolder
        property url backupFolder
        property bool advanced: false
    }

    Label {
        visible: !mobile
        id: title
        font.pointSize: 18
        text: "SACHESI"
        font.letterSpacing: (parent.width / 2) / text.length
        font.weight: Font.DemiBold
        smooth: true
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: font.letterSpacing / 2
    }

    Button {
        anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: 1 }
        height: title.height + 7; width: height
        checkable: true
        checked: settings.advanced
        onClicked: settings.advanced = !settings.advanced
        text: checked ? "+H+" : "H"
        tooltip: qsTr("Advanced")
    }
    TabView {
        id: titleRow
        width: parent.width
        // Qt5.2 bug requires timer to redraw layout correctly
        Timer {
            interval: 10 // Any number works
            running: true
            onTriggered: titleRow.currentIndex = 4
        }

        anchors {top: title.bottom; bottom: parent.bottom }

        Tab {
            title: qsTr("Device")
            Device { anchors.fill: parent; }
        }
        Tab {
            title: qsTr("Extract");
            Extract { anchors.fill: parent }
        }
        Tab {
            title: qsTr("Search")
            Search { anchors.fill: parent }
        }
        Tab {
            title: qsTr("Backup")
            Backup { anchors.fill: parent }
        }
        Tab {
            title: qsTr("Install")
            Installer { anchors.fill: parent }
        }

        USBConnect { anchors.fill: parent }
    }
}
