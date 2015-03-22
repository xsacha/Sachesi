import QtQuick 2.2
import QtQuick.Controls 1.1
import "UI" 1.0

// This page is unix-only as Windows has an outdated driver model which is not compatible

Column {
    id: toolsColumn
    anchors {top: parent.top; topMargin: 20; left: parent.left; leftMargin: 20 }
    width: parent.width - 40;
    spacing: 15
    Text {
        text: qsTr("Boot Communication") + translator.lang
        font.pointSize: 14
        font.bold: true
    }
    Row {
        spacing: 20
        Button {
            text: qsTr("Info") + translator.lang
            onClicked: b.setCommandMode(1, postReboot.checked);
        }
        Button {
            text: qsTr("RimBoot") + translator.lang
            onClicked: b.setCommandMode(2, postReboot.checked);
        }
        Button {
            text: qsTr("Nuke") + translator.lang
            onClicked: b.setCommandMode(3, postReboot.checked);
        }
        Button {
            text: qsTr("Debug Mode") + translator.lang
            onClicked: b.setCommandMode(4, postReboot.checked);
        }
    }
    CheckBox {
        id: postReboot
        text: qsTr("Reboot after") + translator.lang
    }

    Row {
        visible: b.connecting
        spacing: 15
        Text {
            text: qsTr("Connecting to bootrom") + translator.lang
        }
        BusyIndicator {
            height: parent.implicitHeight
            width: height
        }
        Button {
            text: qsTr("Cancel") + translator.lang
            onClicked: b.disconnect();
        }
    }
    GroupBox {
        title: qsTr("Detected devices:") + translator.lang
        ScrollView {
            frameVisible: true
            ListView {
                width: parent.width
                height: 50
                model: b.devices
                spacing: 3

                delegate: Label {
                    text: switch (modelData) {
                        case "1":
                            return "BlackBerry Bootloader";
                        case "8013":
                            return "BlackBerry USB (Unix)";
                        case "8017":
                            return "BlackBerry USB (Autodetect)";
                        default:
                            return "BlackBerry USB (Windows)";
                    }
                    font.pointSize: 12
                }
            }
        }
    }
}
