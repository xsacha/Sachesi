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
            text: "#1. Change PIN (Requires OS10.2)"
            font.pointSize: 12
        }
        Column {
            spacing: 20
            Button {
                text: "Sign NVRAM"
                enabled: i.completed
                onClicked: i.resignNVRAM();
            }
            Text {
                x: 10
                text: "<b>Current PIN</b> " + i.knownPIN
                font.pointSize: 10
            }
            Row {
                spacing: 20
                TextCouple {
                    id: repin
                    type: "New PIN"
                    value: "2CCC0000"
                }
                    Button {
                        text: "Reassign"
                        enabled: i.completed
                        onClicked: i.newPin(repin.value);
                    }
            }
        }*/
        Text {
            text: "#1. Start RTAS (Requires OS10.2)"
            font.pointSize: 12
        }
        Button {
            text: "Start RTAS"
            enabled: i.completed
            onClicked: i.startRTAS();
        }
    }
}
