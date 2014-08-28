import QtQuick 2.2
import "UI" 1.0

PageTab {
    id:main
    property string password: passText.value

    Column {
        id: develText
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -100
        spacing: 10
        Text {
            visible: !i.wrongPassBlock
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Searching for USB device" + (i.possibleDevices ? ("\nTalking to " + i.possibleDevices + " possible devices.") : "")
            font.pixelSize: config.defaultFontSize
        }
        Circle {
            visible: !i.wrongPassBlock
            anchors.horizontalCenter: parent.horizontalCenter
            size: config.defaultFontSize * 2
        }
    }
    TextCouple {
        property bool showing: false
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: develText.bottom; topMargin: 40
        }
        id: passText
        type: "Password"
        property string pass_init: i.password
        value: pass_init;
        onValueChanged: {
            if (i.password !== value && value !== "")
                i.password = value
        }
        textColor: i.wrongPass ? "red" : "black"
        subtext: i.wrongPass ? "Incorrect" : ""
        passMode: showing ? TextInput.Normal : TextInput.Password
    }
    Rectangle {
        width: 30; height: 30
        color: passText.showing ? "gray" : "transparent"
        anchors { left: passText.right; leftMargin: 10; verticalCenter: passText.verticalCenter }
        Image {
            source: "showpass.png"
            anchors.fill: parent
            smooth: true
            MouseArea {
                anchors.fill: parent
                onClicked: passText.showing = !passText.showing
            }
        }
    }

    Row {
        visible: i.wrongPassBlock
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: develText.bottom; topMargin: 80
        }
        Column {
            Text {
                text: "There was an issue connecting."
                font.pixelSize: config.defaultSubtextSize;
            }
            RoundButton {
                text: "Try Again"
                onClicked: i.wrongPassBlock = false
            }
        }
    }


    Text {
        anchors {horizontalCenter: parent.horizontalCenter; bottom: parent.bottom; bottomMargin: 50 }
        text: "These tools require a USB connection"
        font.pixelSize: config.defaultFontSize
    }
}
