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
                Timer { id: try_again; interval: 100; onTriggered: i.wrongPass = false }
                onTextChanged: {
                    if (i.password !== text) {
                        i.password = text
                        if (i.wrongPass)
                            try_again.restart()
                    }
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
            visible: i.loginBlock
            Label {
                text: "There was an issue connecting."
            }
            Button {
                text: "Try Again"
                onClicked: i.loginBlock = false
            }
        }
        RowLayout {
            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
            ColumnLayout {
                Label {
                    visible: !i.loginBlock && !detected.visible
                    text: "Searching for USB device"
                }
                Label {
                    id: detected
                    property int numDevices: typeof b != 'undefined' ? b.devices.length : 0
                    property int deviceType: numDevices ? b.devices[0] : 0
                    property string deviceName: switch(deviceType) {
                                                case 1: return "Bootloader"
                                                case 8012: return "Windows"
                                                case 8013: return "Unix"
                                                default: return ""
                                                }

                    visible: numDevices
                    text: "Detected " + numDevices + " Blackberry USB device(s) in " + deviceName + " mode."
                }
                Label {
                    visible: i.possibleDevices
                    text: "Talking to " + i.possibleDevices + " possible device(s)."
                }
            }
            BusyIndicator {
                visible: !i.loginBlock
                height: parent.implicitHeight + 7
                width: height
            }
        }
    }
}
