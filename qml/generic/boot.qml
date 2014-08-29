import QtQuick 2.2
import QtQuick.Controls 1.2
import "UI" 1.0

PageTab {
    id: main
    Column {
        id: toolsColumn
        anchors {top: parent.top; topMargin: 20; left: parent.left; leftMargin: 20 }
        width: parent.width - 40;
        spacing: 15
        Text {
            text: "Boot Communication"
            font.pixelSize: config.defaultFontSize
            font.bold: true
        }
        Row {
            spacing: 20
            RoundButton {
                text: "Info"
                onClicked: b.setCommandMode(1, postReboot.checked);
            }
            RoundButton {
                text: "RimBoot"
                onClicked: b.setCommandMode(2, postReboot.checked);
            }
            RoundButton {
                text: "Nuke"
                onClicked: b.setCommandMode(3, postReboot.checked);
            }
            RoundButton {
                text: "Debug Mode"
                onClicked: b.setCommandMode(4, postReboot.checked);
            }
        }
        CheckBox {
            id: postReboot
            text: "Reboot after"
        }

        Row {
            visible: b.connecting
            spacing: 15
            Text {
                text: "Connecting to bootrom"
                font.pixelSize: config.notificationFontSize

            }
            BusyIndicator {
                width: config.notificationFontSize
                height: config.notificationFontSize
            }
            RoundButton {
                text: "Cancel"
                onClicked: b.disconnect();
            }
        }
        ListView {
            width: parent.width
            height: 50
            model: b.devices
            spacing: 3
            clip: true
            header: Text {
                width: parent.width
                height: config.defaultFontSize
                text: "Detected devices"
                font.bold: true
                font.pixelSize: config.defaultFontSize
            }

            delegate: Text {
                width: parent.width
                height: config.defaultFontSize
                text: "Blackberry " + (modelData == "1" ? "Bootloader" : ((modelData == "8013") ? "USB (Unix)" : "USB (Windows)"));
                font.pixelSize: config.defaultFontSize
            }
        }
    }

    Text {
        anchors {bottom: parent.bottom; bottomMargin: 100; horizontalCenter: parent.horizontalCenter }
        font.pixelSize: config.notificationFontSize + 6
    }
}
