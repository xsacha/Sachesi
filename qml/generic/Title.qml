import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Window 2.1
import Qt.labs.settings 1.0
import "UI" 1.0

ApplicationWindow {
    id: window
    title: qsTr("Sachesi") + " " + version
    width: 820
    height: 680
    minimumHeight: 540
    minimumWidth: 620

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
        font.pointSize: 22
        text:  qsTr("SACHESI")
        font.letterSpacing: (parent.width - 280) / text.length
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
        currentIndex: 1 + p.hasBootAccess
        width: parent.width
        anchors {top: title.bottom; bottom: parent.bottom }
        // Workaround for index moving on startup for Windows
        onCountChanged: { titleRow.currentIndex = 0; if (count >= 1 + p.hasBootAccess) titleRow.currentIndex = 1 + p.hasBootAccess; }

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

        Component.onCompleted: {
            if (p.hasBootAccess)
                titleRow.addTab(qsTr("Boot"), Qt.createComponent("Boot.qml") )
        }

        USBConnect { anchors.fill: parent }
    }

    statusBar: StatusBar {
        visible: !mobile
        Label {
            visible: i.knownBattery < 0
            text:  qsTr("No device connected")
        }
        Label {
            visible: i.knownBattery > -1
            property bool hasSpace: window.width > 700
            text:  "<b>[</b>USB" + " ("+i.knownBattery+"%)<b>]</b>  " + "  <b>[</b>OS:" + i.knownOS + " Radio:" + i.knownRadio + "<b>]</b>" + (hasSpace ? (" <b>[</b>" + i.knownName + "<b>]</b>") : "");
        }
        Label {
            visible: i.knownBattery > -1
            anchors.right: parent.right
            text:  "<b>[</b>"+i.knownHW+"<b>]</b>"
        }
    }
}
