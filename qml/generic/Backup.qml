import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.1
import Qt.labs.settings 1.0
import BackupTools 1.0
import "UI" 1.0


Item {
    id: main
    visible: i.device !== null && i.completed && !i.loginBlock && !i.wrongPass
    anchors { fill: parent; leftMargin: 20; topMargin: 20 }

    MessageDialog {
        id: timeoutWarning
        title: qsTr("Timed out")
        informativeText: qsTr("The request timed out. Please backup blind. This is a bug that Blackberry needs to fix.")
        standardButtons: StandardButton.Ok
    }
    Timer {
        id: attemptLookup
        running: false
        interval: 40000
        onTriggered: timeoutWarning.open()
    }

    ColumnLayout {
        ColumnLayout {
            Text {
                text: qsTr("Options") + translator.lang
                font.pointSize: 14
                font.bold: true
            }
            id: options
            property int value: 0
            RowLayout {
                visible: attemptLookup.running
                Text {
                    text: qsTr("Refreshing backup sizes") + translator.lang
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
                        text: (i.backNames[index] + " (" + (i.backSizes[index] < 0 ? qsTr("Unknown Size") : qsTr("%1 MB").arg(i.backSizes[index].toFixed(1))) + ")") + translator.lang // index
                        onTextChanged: if (attemptLookup.running) attemptLookup.stop()
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
                text: (qsTr("Total:") + " " + (totalVal < 0 ? qsTr("Unknown Size") : qsTr("%1 MB").arg(totalVal.toFixed(1)))) + translator.lang
                font.pointSize: 12
            }
        }

        Button {
            visible: /*!i.backMethods &&*/ !attemptLookup.running
            enabled: !i.installing && !i.backing && !i.restoring && i.device !== null && i.device.bbid !== ""
            text:  qsTr("Refresh Backup Sizes") + translator.lang
            onClicked: { totalText.totalVal = 0; i.backupQuery(); attemptLookup.start(); }
        }

        Label {
            visible: !settings.advanced
            text: qsTr("Loading backup sizes can sometimes fail. In this situation, you can backup 'blind'.") + translator.lang
        }
        RowLayout {
            visible: i.backMethods
            FileDialog {
                id: backup_files
                title:  qsTr("Choose Backup Filename") + translator.lang
                folder: settings.backupFolder
                onAccepted: {
                    i.backup(fileUrl, options.value)
                    settings.backupFolder = folder;
                }
                selectExisting: false

                nameFilters: [ qsTr("Blackberry Backup (*.bbb)") + translator.lang ]
            }
            FileDialog {
                id: restore_files
                title:  qsTr("Select Restore File") + translator.lang
                folder: settings.backupFolder
                onAccepted: {
                    i.restore(fileUrl, options.value)
                    settings.backupFolder = folder;
                }

                nameFilters: [ qsTr("Blackberry Backup (*.bbb)") + translator.lang ]
            }

            Button {
                text: (totalText.totalVal < 0 ? qsTr("Create Backup Blind") : qsTr("Create Backup")) + translator.lang
                enabled: !i.installing && !i.backing && !i.restoring && options.value != 0 && i.device !== null && i.device.bbid !== ""
                onClicked: backup_files.open();
            }
            Button {
                text:  qsTr("Restore Backup") + translator.lang
                enabled: !i.installing && !i.backing && !i.restoring && options.value != 0 && i.device !== null && i.device.bbid !== ""
                onClicked: restore_files.open();
            }
        }
        Label {
            visible: i.device !== null && i.device.bbid === ""
            text: qsTr("Your device needs a Blackberry ID to perform backups or restores!") + translator.lang
        }
        Label {
            visible: !settings.advanced
            text: qsTr("Please note that backups can take a long time, depending on your device data.") + translator.lang
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
                text: (i.backing ? qsTr("Creating Backup (%1%)").arg(i.backProgress) : qsTr("Restoring Backup (%1%)").arg(i.backProgress)) + translator.lang
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
            Text {
                font.pointSize: 10
                visible: i.backProgress == 0
                text: "The device is currently generating a list of files to backup. Please wait."
            }
        }
    }
}
