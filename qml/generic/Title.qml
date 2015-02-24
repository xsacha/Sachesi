import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Window 2.1
import Qt.labs.settings 1.0
import "UI" 1.0

ApplicationWindow {
    id: window
    title: "Sachesi " + version + " Test"
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
        property bool nativelang: true
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
        tooltip: qsTr("Advanced") + translator.lang
    }

    ComboBox {
        model: [ Qt.locale().nativeLanguageName, "English" ]
        anchors { right: parent.right; top: parent.top; topMargin: 1 }
        visible: translator.exists
        currentIndex: settings.nativelang ? 0 : 1

        onCurrentIndexChanged: {
            if (currentIndex === 0) {
                translator.load();
                settings.nativelang = true;
            } else {
                translator.remove();
                settings.nativelang = false;
            }
        }
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
            title: qsTr("Device") + translator.lang
            Device { anchors.fill: parent; }
        }
        Tab {
            title: qsTr("Extract") + translator.lang
            Extract { anchors.fill: parent }
        }
        Tab {
            title: qsTr("Search") + translator.lang
            Search { anchors.fill: parent }
        }
        Tab {
            title: qsTr("Backup") + translator.lang
            Backup { anchors.fill: parent }
        }
        Tab {
            title: qsTr("Install") + translator.lang
            Installer { anchors.fill: parent }
        }

        USBConnect { anchors.fill: parent }
    }
}
