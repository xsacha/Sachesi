import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import QtQuick.Window 2.1


Item {
    id: main
    ColumnLayout {
        anchors { fill: parent; margins: 15 }
        Label {
            text: qsTr("Device Information") + translator.lang
            font.pointSize: 14
            font.bold: true
        }
        GroupBox {
            visible: i.device !== null
            title:  qsTr("Tools") + translator.lang
            RowLayout {
                Button {
                    id: wipe
                    text:  qsTr("Wipe") + translator.lang
                    onClicked: i.wipe();
                }
                Button {
                    id: factorywipe
                    text:  qsTr("Factory Reset") + translator.lang
                    onClicked: i.factorywipe();
                }
                Button {
                    id: reboot
                    text:  qsTr("Reboot") + translator.lang
                    onClicked: i.reboot();
                }
            }
        }

        GridLayout {
            columns: 4
            rowSpacing: 20
            columnSpacing: 20

            Label {
                text: qsTr("Name") + translator.lang
                font.bold: true
            }
            Label {
                id: deviceNameText
                text: i.device === null ? (qsTr("Unknown") + translator.lang) : i.device.friendlyName
            }
            Label {
                text: qsTr("HW Name") + translator.lang
                font.bold: true
            }
            Label {
                text: i.device === null ? (qsTr("Unknown") + translator.lang) : i.device.name
                font.pointSize: i.device === null ? -1 : 10
            }

            Label {
                text: qsTr("BBID") + translator.lang
                font.bold: true
            }
            Label {
                text: i.device === null ? (qsTr("Unknown") + translator.lang) : i.device.bbid
            }
            Label {
                text: qsTr("PIN") + translator.lang
                font.bold: true
            }
            Label {
                text: i.device === null ? (qsTr("Unknown") + translator.lang) : i.device.pin
            }

            Label {
                text: qsTr("BSN") + translator.lang
                font.bold: true
            }
            Label {
                text: i.device === null ? (qsTr("Unknown") + translator.lang) : i.device.bsn
            }
            Label {
                font.bold: true
            }
            Label {

            }

            Label {
                text: qsTr("OS") + translator.lang
                font.bold: true
            }
            Label {
                text: i.device === null ? (qsTr("Unknown") + translator.lang) : i.device.os
            }

            Label {
                text: qsTr("Radio") + translator.lang
                font.bold: true
            }
            Label {
                text: i.device === null ? (qsTr("Unknown") + translator.lang) : i.device.radio
            }

            Label {
                text: qsTr("HW") + translator.lang
                font.bold: true
            }
            Label {
                text: i.device === null ? (qsTr("Unknown") + translator.lang) : i.device.hw
            }

            Label {
                text: qsTr("Restrictions") + translator.lang
                font.bold: true
            }
            Label {
                text: (i.device === null ? qsTr("Unknown") : (i.device.restrictions === "" ? qsTr("None") : i.device.restrictions)) + translator.lang
            }

            Label {
                text: qsTr("Setup Complete") + translator.lang
                font.bold: true
            }
            Label {
                text: (i.device === null ? qsTr("Unknown") : (i.device.setupComplete ? qsTr("True") : qsTr("False"))) + translator.lang
            }

            Label {
                text: qsTr("Developer Mode") + translator.lang
                font.bold: true
            }
            Label {
                text: (i.device === null ? qsTr("Unknown") : (i.device.devMode ? qsTr("True") : qsTr("False"))) + translator.lang
            }

            Label {
                text: qsTr("Battery") + translator.lang
                font.bold: true
            }
            Label {
                text: (i.device === null || i.device.battery < 0) ? (qsTr("Unknown") + translator.lang) : i.device.battery + "%"
            }

            Label {
                text: qsTr("Connection") + translator.lang
                font.bold: true
            }
            Label {
                text: ((i.device === null || i.device.battery < 0) ? qsTr("None") : qsTr("USB")) + translator.lang
            }

            Label {
                text: qsTr("Refurbished Date") + translator.lang
                font.bold: true
            }
            RowLayout {
                Label {
                    id: refurbText
                    text: ((i.device === null) ? qsTr("Unknown") : (i.device.refurbDate === "" ? qsTr("Never") : i.device.refurbDate)) + translator.lang
                }
                Button {
                    property bool isSet: refurbText.text !== (qsTr("Never") + translator.lang)
                    visible: refurbText.text !== (qsTr("Unknown") + translator.lang)
                    text: (isSet ? qsTr("Clear") : qsTr("Set")) + translator.lang
                    onClicked: {
                        var date = Math.floor(new Date().getTime() / 1000)
                        i.setActionProperty("RefurbDate", isSet ? "0" : date.toString())
                        updateProps.start()
                    }
                    Timer {
                        id: updateProps
                        interval: 100
                        onTriggered: i.scanProps()
                    }
                }
            }

            Label {
                text: qsTr("Free Disk Space") + translator.lang
                font.bold: true
            }
            Label {
                text: ((i.device === null || i.device.freeSpace === 0) ? qsTr("Unknown") : qsTr("%1 GB").arg((i.device.freeSpace / 1024 / 1024 / 1024).toFixed(3))) + translator.lang
            }
        }
    }

}
