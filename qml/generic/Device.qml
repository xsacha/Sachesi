import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import QtQuick.Window 2.1


Item {
    id: main
    ColumnLayout {
        anchors { fill: parent; margins: 15 }
        Layout.fillHeight: true
        Layout.fillWidth: true
        Label {
            text: qsTr("Device Information")
            font.pointSize: 14
            font.bold: true
        }
        GroupBox {
            title:  qsTr("Tools")
            RowLayout {
                Button {
                    id: wipe
                    text:  qsTr("Wipe")
                    onClicked: i.wipe();
                }
                Button {
                    id: factorywipe
                    text:  qsTr("Factory Reset")
                    onClicked: i.factorywipe();
                }
                Button {
                    id: reboot
                    text:  qsTr("Reboot")
                    onClicked: i.reboot();
                }
            }
        }

        GridLayout {
            columns: 4
            rowSpacing: 20
            columnSpacing: 20

            Label {
                text: qsTr("Name")
                font.bold: true
            }
            Label {
                id: deviceNameText
                text: i.device === null ? qsTr("Unknown") : i.device.friendlyName
            }
            Label {
                text: qsTr("HW Name")
                font.bold: true
            }
            Label {
                text: i.device === null ? qsTr("Unknown") : i.device.name
                font.pointSize: i.device === null ? -1 : 10
            }

            Label {
                text: qsTr("BBID")
                font.bold: true
            }
            Label {
                text: i.device === null ? qsTr("Unknown") : i.device.bbid
            }
            Label {
                text: qsTr("PIN")
                font.bold: true
            }
            Label {
                text: i.device === null ? qsTr("Unknown") : i.device.pin
            }

            Label {
                text: qsTr("OS")
                font.bold: true
            }
            Label {
                text: i.device === null ? qsTr("Unknown") : i.device.os
            }

            Label {
                text: qsTr("Radio")
                font.bold: true
            }
            Label {
                text: i.device === null ? qsTr("Unknown") : i.device.radio
            }

            Label {
                text: qsTr("HW")
                font.bold: true
            }
            Label {
                text: i.device === null ? qsTr("Unknown") : i.device.hw
            }

            Label {
                text: qsTr("Restrictions")
                font.bold: true
            }
            Label {
                text: i.device === null ? qsTr("Unknown") : (i.device.restrictions === "" ? qsTr("None") : i.device.restrictions)
            }

            Label {
                text: qsTr("Setup Complete")
                font.bold: true
            }
            Label {
                text: i.device === null ? qsTr("Unknown") : (i.device.setupComplete ? qsTr("True") : qsTr("False"))
            }

            Label {
                text: qsTr("Developer Mode")
                font.bold: true
            }
            Label {
                text: i.device === null ? qsTr("Unknown") : (i.device.devMode ? qsTr("True") : qsTr("False"))
            }

            Label {
                text: qsTr("Battery")
                font.bold: true
            }
            Label {
                text: (i.device === null || i.device.battery < 0) ? qsTr("Unknown") : i.device.battery + "%"
            }

            Label {
                text: qsTr("Connection")
                font.bold: true
            }
            Label {
                text: (i.device === null || i.device.battery < 0) ? qsTr("Unknown") : qsTr("USB")
            }

            Label {
                text: qsTr("Refurbished Date")
                font.bold: true
            }
            RowLayout {
                Label {
                    id: refurbText
                    text: (i.device === null) ? qsTr("Unknown") : (i.device.refurbDate === "" ? qsTr("Never") : i.device.refurbDate)
                }
                Button {
                    visible: refurbText.text !== qsTr("Unknown") && refurbText.text !== qsTr("Never")
                    text: qsTr("Clear")
                    onClicked: {
                        i.setActionProperty("RefurbDate", "0")
                        i.device.refurbDate = "";
                    }
                }
            }
        }
    }

}
