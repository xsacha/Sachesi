import QtQuick 2.2
import QtQuick.Controls 1.1
import "UI" 1.0

Column {
    id: toolsColumn
    anchors {top: parent.top; topMargin: 20; left: parent.left; leftMargin: 20 }
    width: parent.width - 40;
    spacing: 15
    Text {
        text: "Boot Communication"
        font.pointSize: 14
        font.bold: true
    }
    Row {
        spacing: 20
        Button {
            text: "Info"
            onClicked: b.setCommandMode(1, postReboot.checked);
        }
        Button {
            text: "RimBoot"
            onClicked: b.setCommandMode(2, postReboot.checked);
        }
        Button {
            text: "Nuke"
            onClicked: b.setCommandMode(3, postReboot.checked);
        }
        Button {
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
        }
        BusyIndicator {
            height: parent.implicitHeight
            width: height
        }
        Button {
            text: "Cancel"
            onClicked: b.disconnect();
        }
    }
    GroupBox {
        title: "Detected devices:"
        ScrollView {
            frameVisible: true
            ListView {
                width: parent.width
                height: 50
                model: b.devices
                spacing: 3

                delegate: Label {
                    text: "Blackberry " + (modelData == "1" ? "Bootloader" : ((modelData == "8013") ? "USB (Unix)" : "USB (Windows)"));
                    font.pointSize: 12
                }
            }
        }
    }
}
