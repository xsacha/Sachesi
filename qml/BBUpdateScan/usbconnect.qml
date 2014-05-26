// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Rectangle {
    id: main
    width: 480
    height: 340
    radius: 8
    SystemPalette {id: pal}
    border.width: 2
    border.color: pal.shadow
    gradient: Gradient {
        GradientStop { position: 0.0; color: pal.mid }
        GradientStop { position: 1.0; color: pal.midlight }
    }

    //property string ip: ipText.value
    property string password: passText.value
    //property string ip_init: i.ip
    property string pass_init: i.password

    /*Row {
        anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: 20 }
        //anchors { left: parent.left; leftMargin: 20; top: parent.top; topMargin: 20 }
        spacing: 20
        TextCouple {
            id: ipText
            type: "IP Address"
            value: ip_init
            readOnly: true
            textColor: "grey"
            before:passText
            after:passText
            onValueChanged: {
                var tmpText = "";
                for (var i = 0; i < value.length; i++)
                {
                    var c = value.charAt(i);
                    if ((c < '0' || c > '9') && c != '.')
                        continue;
                    else
                    tmpText += value.charAt(i);
                }
                value = tmpText;
                if (i.ip !== value) i.ip = value
            }
        }
    }*/

    Text {
        id: develText
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -100
        anchors.horizontalCenterOffset: -27
        text: "Searching for USB device"
        font.pixelSize: 14
    }
    Image {
        id: circle
        anchors {
            verticalCenter: develText.verticalCenter
            left: develText.right; leftMargin: 10
        }
        source: "circle.png";
        SequentialAnimation {
            running: true
            NumberAnimation { target: circle; property: "rotation"; alwaysRunToEnd: true; duration: 1000; from: 0; to: 360; }
            onRunningChanged: running = true;
        }
        width: 43; height: 45
    }
    TextCouple {
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: develText.bottom; topMargin: 40
        }
        id: passText
        type: "Password"
        value: pass_init
        //before:ipText
        //after:ipText
        onValueChanged: {
            if (i.password !== value)
                i.password = value
            i.wrongPass = false
        }
        textColor: i.wrongPass ? "red" : "black"
        subtext: i.wrongPass ? "Incorrect" : ""
    }
    Text {
        anchors {horizontalCenter: parent.horizontalCenter; bottom: parent.bottom; bottomMargin: 50 }
        text: "These tools require a USB connection"
        font.pixelSize: 14
    }
}
