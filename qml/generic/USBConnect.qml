import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

Item {
    id:main
    visible: titleRow.currentIndex > (1 + p.hasBootAccess) && i.knownBattery < 0
    anchors.fill: parent

    ColumnLayout {
        id: develText
        anchors.centerIn: parent
        height: parent.height / 2
        Label {
            Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
            text: "These tools require a USB connection"
        }
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            ColumnLayout {
                Label {
                    text: "Password:"
                }
                Label {
                    id: subtextValue
                    visible: i.wrongPass
                    text: "Incorrect"
                    color: "red"
                }
            }
            TextField {
                id: passText
                property bool showing: false
                property string pass_init: i.password
                text: pass_init;
                onTextChanged: {
                    if (i.password !== text)
                        i.password = text
                }
                echoMode: showing ? TextInput.Normal : TextInput.Password
            }
            Button {
                tooltip: passText.showing ? "Hide password" : "Show password"
                anchors { left: passText.right; leftMargin: 10; verticalCenter: passText.verticalCenter }
                iconSource: "showpass.png"
                onClicked: passText.showing = !passText.showing
            }
        }
        ColumnLayout {
            visible: i.wrongPassBlock
            Label {
                text: "There was an issue connecting."
            }
            Button {
                text: "Try Again"
                onClicked: i.wrongPassBlock = false
            }
        }
        RowLayout {
            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
            ColumnLayout {
                Label {
                    visible: !i.wrongPassBlock
                    text: "Searching for USB device"
                }
                Label {
                    visible: i.possibleDevices
                    text: "Talking to " + i.possibleDevices + " possible devices."
                }
            }
            BusyIndicator {
                visible: !i.wrongPassBlock
                height: parent.implicitHeight + 7
                width: height
            }
        }
    }
}
