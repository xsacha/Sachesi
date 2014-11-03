import QtQuick 2.2
import QtQuick.Controls 1.1
import "UI" 1.0

// This page is hidden for now as it is not very useful.

TabView {
    id: main
    Column {
        anchors {left: parent.left; leftMargin: 20; top: parent.top; topMargin: 20 }
        spacing: 20

        /*
        Text {
            text: qsTr("#1. Change PIN (Requires OS10.2)") + translator.lang
            font.pointSize: 12
        }
        Column {
            spacing: 20
            Button {
                text:  qsTr("Sign NVRAM") + translator.lang
                enabled: i.completed
                onClicked: i.resignNVRAM();
            }
            Text {
                x: 10
                text: qsTr("<b>Current PIN</b> %1").arg(i.knownPIN) + translator.lang
                font.pointSize: 10
            }
            Row {
                spacing: 20
                TextCouple {
                    id: repin
                    type: qsTr("New PIN") + translator.lang
                    value: "2CCC0000"
                }
                    Button {
                        text:  qsTr("Reassign") + translator.lang
                        enabled: i.completed
                        onClicked: i.newPin(repin.value);
                    }
            }
        }*/
        Text {
            text: qsTr("#1. Start RTAS (Requires OS10.2)") + translator.lang
            font.pointSize: 12
        }
        Button {
            text: qsTr("Start RTAS") + translator.lang
            enabled: i.completed
            onClicked: i.startRTAS();
        }
    }
}
