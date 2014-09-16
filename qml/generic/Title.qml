import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Window 2.1
import Qt.labs.settings 1.0
import "UI" 1.0

ApplicationWindow {
    id: window
    // TODO: Send version from C++
    title: "Sachesi " + version
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

    Text {
        id: title
        font.pointSize: 22
        text: "SACHESI"
        font.letterSpacing: (parent.width - 280) / text.length
        font.weight: Font.DemiBold
        smooth: true
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: font.letterSpacing / 2
    }

    Rectangle {
        id: advanced_toggle
        anchors { right: parent.right; rightMargin: 1; top: parent.top; topMargin: 1 }
        height: title.height; width: height
        radius: height
        color: settings.advanced ? "#999999" : "transparent"
        border.width: 2
        antialiasing: true
        MouseArea {
            anchors.fill: parent
            onClicked: settings.advanced = !settings.advanced
        }
    }
    TabView {
        id: titleRow
        currentIndex: 1 + p.hasBootAccess
        width: parent.width
        anchors {top: title.bottom; bottom: parent.bottom }
        // Workaround for index moving on startup for Windows
        onCountChanged: { titleRow.currentIndex = 0; if (count > p.hasBootAccess) titleRow.currentIndex = 1 + p.hasBootAccess; }

        Tab {
            title: "Extract";
            Extract { anchors.fill: parent }
        }
        Tab {
            title: "Search"
            Search { anchors.fill: parent
                Component.onCompleted: if (!blackberry) {
                    titleRow.addTab("Backup", Qt.createComponent("Backup.qml") )
                    titleRow.addTab("Install", Qt.createComponent("Installer.qml") )
                }
            }
        }
        Component.onCompleted: {
            if (p.hasBootAccess)
                titleRow.addTab("Boot", Qt.createComponent("Boot.qml") )
        }

        USBConnect { anchors.fill: parent }
    }
    /*Rectangle {
        visible: !hasDonated
        width: parent.width; height: 30
        anchors.bottom: parent.bottom
        Label {
            anchors.centerIn: parent
            text: "If you enjoy using this tool. Please consider donating."
            MouseArea {
                anchors.fill: parent;
                onClicked: {
                    hasDonated = true
                    Qt.openUrlExternally("https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=xsacha@gmail.com&lc=AU&item_name=Time+and+effort&item_number=xsacha&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHosted")
                }
            }
        }
    }*/
    statusBar: StatusBar {
        Label {
            visible: i.knownBattery < 0
            text: "No device connected"
        }
        Label {
            visible: i.knownBattery > -1
            property bool hasSpace: window.width > 700
            text: "<b>[</b>USB" + " ("+i.knownBattery+"%)<b>]</b>  " + "  <b>[</b>OS:" + i.knownOS + " Radio:" + i.knownRadio + "<b>]</b>" + (hasSpace ? (" <b>[</b>" + i.knownName + "<b>]</b>") : "");
        }
        Label {
            visible: i.knownBattery > -1
            anchors.right: parent.right
            text: "<b>[</b>"+i.knownHW+"<b>]</b>"
        }
    }
}
