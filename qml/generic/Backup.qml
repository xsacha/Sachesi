import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.1
import Qt.labs.settings 1.0
import BackupTools 1.0
import "UI" 1.0


Item {
    id: main
    visible: i.device !== null && !i.loginBlock && !i.wrongPass
    anchors { fill: parent; leftMargin: 20; topMargin: 20 }

    Timer {
        id: attemptLookup
        running: false
        interval: 22000
    }

    ColumnLayout {
        ColumnLayout {
            Text {
                text:  qsTr("Options")
                font.pointSize: 14
                font.bold: true
            }
            id: options
            property int value: 0
            RowLayout {
                visible: attemptLookup.running
                Text {
                    text:  qsTr("Refreshing Backup Sizes")
                    font.pointSize: 12
                }
                BusyIndicator {
                    width: parent.height
                    height: parent.height
                }
            }
            ColumnLayout {
                Repeater {
                    model: i.backMethods
                    delegate: CheckBox {
                        text: i.backNames[index] + " (" + (i.backSizes[index] < 0 ? qsTr("Unknown") : i.backSizes[index].toFixed(1)) + qsTr(" MB)") // index
                        onCheckedChanged: {
                            if (checked) {
                                options.value += 1 << index;
                                totalText.totalVal += i.backSizes[index];
                            } else {
                                options.value -= 1 << index;
                                totalText.totalVal -= i.backSizes[index]
                            }
                        }
                    }
                }
            }
            Text {
                visible: i.backMethods
                id: totalText
                property double totalVal: 0.0
                text:  qsTr("Total: ") + (totalVal < 0 ? qsTr("Unknown") : totalVal.toFixed(1)) + qsTr(" MB")
                font.pointSize: 12
            }
        }

        Button {
            visible: /*!i.backMethods &&*/ !attemptLookup.running
            enabled: !i.installing && !i.backing && !i.restoring && i.device !== null && i.device.bbid !== ""
            text:  qsTr("Refresh Backup Sizes")
            onClicked: { totalText.totalVal = 0; i.backupQuery(); attemptLookup.start(); }
        }

        Label {
            visible: !settings.advanced
            text: qsTr("Loading backup sizes can sometimes fail. In this situation, you can backup 'Blind'.")
        }
        RowLayout {
            visible: i.backMethods
            FileDialog {
                id: backup_files
                title:  qsTr("Choose backup filename")
                folder: settings.backupFolder
                onAccepted: {
                    i.backup(fileUrl, options.value)
                    settings.backupFolder = folder;
                }
                selectExisting: false

                nameFilters: [ qsTr("Blackberry Backup (*.bbb)") ]
            }
            FileDialog {
                id: restore_files
                title:  qsTr("Select restore file")
                folder: settings.backupFolder
                onAccepted: {
                    i.restore(fileUrl, options.value)
                    settings.backupFolder = folder;
                }

                nameFilters: [ "Blackberry Backup (*.bbb)" ]
            }

            Button {
                text:  qsTr("Create backup") + (totalText.totalVal < 0 ? (" " + qsTr("Blind")) : "")
                enabled: !i.installing && !i.backing && !i.restoring && options.value != 0 && i.device !== null && i.device.bbid !== ""
                onClicked: backup_files.open();
            }
            Button {
                text:  qsTr("Restore backup")
                enabled: !i.installing && !i.backing && !i.restoring && options.value != 0 && i.device !== null && i.device.bbid !== ""
                onClicked: restore_files.open();
            }
        }
        Label {
            visible: i.device !== null && i.device.bbid === ""
            text: qsTr("Your device needs a Blackberry ID to perform backups or restores!")
        }
        Label {
            visible: !settings.advanced
            text: qsTr("Please note that backups can take a long time, depending on your device data.")
        }
    }
    Rectangle {
        id: progressBar
        visible: i.backing || i.restoring
        anchors {bottom: parent.bottom; bottomMargin: 20; horizontalCenter: parent.horizontalCenter }
        height: 66; width: parent.width - parent.width / 4; radius: 8
        z: 5;
        color: "gray"
        opacity: 0.95
        Column {
            anchors {verticalCenter: parent.verticalCenter; left: parent.left; leftMargin: parent.width / 2 - 150 }
            Text {
                font.pointSize: 12
                text: (i.backing ? qsTr("Creating") : qsTr("Restoring")) + qsTr(" Backup (") + i.backProgress + "%)";
            }
            Row {
                spacing: 10
                Text {
                    font.pointSize: 12
                    text: i.backStatus + " (" + i.backCurProgress + "%)";
                }
                BusyIndicator {
                    width: parent.height
                    height: parent.height
                }
            }
        }
    }
}
