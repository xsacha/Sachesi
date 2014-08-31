import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Window 2.2
// This might be a tough sell. It's very nice but not included in package
import Qt.labs.settings 1.0
import "UI" 1.0

ApplicationWindow {
    id: window
    // TODO: Send version from C++
    title: "Sachesi 1.5.0"
    width: 520
    height: 480
    minimumHeight: 520
    minimumWidth: 440

    Settings {
        property alias x: window.x
        property alias y: window.y
        property alias width: window.width
        property alias height: window.height
        property alias tab: titleRow.currentIndex
    }

    Config {id:config}

    Text {
        id: title
        font.pixelSize: 22
        text: "SACHESI"
        font.letterSpacing: (parent.width - 280) / 6
        font.weight: Font.DemiBold
        smooth: true
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: (parent.width - 280) / 12
    }

    Rectangle {
        anchors { right: parent.right; rightMargin: 1; top: parent.top; topMargin: 1 }
        width: 28; height: 28
        radius: height
        color: p.advanced ? "#999999" : "transparent"
        border.width: 2
        smooth: true
        MouseArea {
            anchors.fill: parent
            onClicked: p.advanced = !p.advanced
        }
    }
    TabView {
        id: titleRow
        width: parent.width
        anchors {top: title.bottom; bottom: parent.bottom }
        currentIndex: 3
        Component.onCompleted: {
            addTab("Extract", Qt.createComponent("extract.qml"));
            //if (p.hasBootAccess)
            //    addTab("Tools", Qt.createComponent("downloader.qml"));
            if (p.hasBootAccess)
                addTab("Boot", Qt.createComponent("boot.qml"));
            addTab("Search", Qt.createComponent("main.qml"));
            addTab("Backup", Qt.createComponent("backup.qml"));
            addTab("Install", Qt.createComponent("installer.qml"));
        }
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
        visible: i.knownBattery > -1
        Label {
            property bool hasSpace: window.width > 700
            text: "<b>[</b>USB" + " ("+i.knownBattery+"%)<b>]</b>  " + "  <b>[</b>OS:" + i.knownOS + " Radio:" + i.knownRadio + "<b>]</b>" + (hasSpace ? (" <b>[</b>" + i.knownName + "<b>]</b>") : "");
        }
        Label {
            anchors.right: parent.right
            text: "<b>[</b>"+i.knownHW+"<b>]</b>"
        }
    }
}
