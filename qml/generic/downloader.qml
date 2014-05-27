import QtQuick 1.1
import "UI" 1.0

PageTab {
    id:main
    Column {
        anchors {left: parent.left; leftMargin: 20; top: parent.top; topMargin: 20 }
        spacing: 20

        /*
        Text {
            text: "#1. Change PIN (Requires OS10.2)"
            font.pixelSize: config.defaultFontSize
        }
        Column {
            spacing: 20
            RoundButton {
                text: "Sign NVRAM"
                enabled: i.completed
                onClicked: i.resignNVRAM();
            }
            Text {
                x: 10
                text: "<b>Current PIN</b> " + i.knownPIN
                font.pixelSize: config.defaultButtonTextSize
            }
            Row {
                spacing: 20
                TextCouple {
                    id: repin
                    type: "New PIN"
                    value: "2CCC0000"
                }
                    RoundButton {
                        text: "Reassign"
                        enabled: i.completed
                        onClicked: i.newPin(repin.value);
                    }
            }
        }*/
        Text {
            text: "#1. Start RTAS (Requires OS10.2)"
            font.pixelSize: config.defaultFontSize
        }
        RoundButton {
            text: "Start RTAS"
            enabled: i.completed
            onClicked: i.startRTAS();
        }
    }
}
