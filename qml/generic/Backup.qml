import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.1
import QtQuick.Window 2.1
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
                    onCountChanged: options.value = 0
                    model: i.backMethods
                    delegate: CheckBox {
                        property double curSize: i.backSizes[index]
                        text: (i.backNames[index] + " (" + (i.backSizes[index] < 0 ? qsTr("Unknown Size") : qsTr("%1 MB").arg( ((index == 0 ? appData.selectedSize : i.backSizes[index]) / 1024 / 1024).toFixed(1))) + ")") + translator.lang // index
                        onCurSizeChanged: if (index == 0) appData.selectedSize = curSize
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
                        Button {
                            anchors.left: parent.right
                            visible: index == 0 && i.backSizes[0] > 0 && !appDataSelect.visible
                            text: qsTr("Choose") + translator.lang
                            onClicked: appDataSelect.show()
                        }
                    }
                }
            }
            Window {
                id: appDataSelect
                width: 600
                height: 600
                title: qsTr("Choose Application Data") + translator.lang
                onVisibleChanged: if (visible) {
                                      x = window.x + (window.width - width) / 2
                                      y = window.y + (window.height - height) / 2
                                  }
                ColumnLayout {
                    anchors.fill: parent
                    Text {
                        text: qsTr("Total Application Data: %1 MB (%2 Apps)").arg(i.backMethods > 0 ? (i.backSizes[0] / 1024 / 1024).toFixed(1) : "0").arg(backAppView.count) + translator.lang
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                    }
                    Text {
                        id: appData
                        Layout.fillWidth: true
                        property double selectedSize: -1.0
                        text: qsTr("Selected Application Data: %1 MB").arg((selectedSize / 1024 / 1024).toFixed(1)) + translator.lang
                        wrapMode: Text.Wrap
                    }

                    Component {
                        id: sectionHeading
                        RowLayout{
                            CheckBox {
                                checked: true
                            }

                            Rectangle {
                                //width: container.width
                                height: childrenRect.height
                                color: "lightsteelblue"

                                Text {
                                    text: section
                                    font.bold: true
                                    font.pixelSize: 20
                                }
                            }
                        }
                    }
                    ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        ListView {
                            anchors.fill: parent
                            id: backAppView
                            model: i.backAppList
                            section.property: "type"
                            section.criteria: ViewSection.FullString
                            section.delegate: sectionHeading
                            Menu {
                                id: back_options_menu
                                signal checkAll()
                                signal uncheckAll()
                                title:  qsTr("Options") + translator.lang
                                MenuItem {
                                    text:  qsTr("Check All Visible") + translator.lang
                                    onTriggered: {
                                        back_options_menu.checkAll();
                                    }
                                }
                                MenuItem {
                                    text:  qsTr("Uncheck All Visible") + translator.lang
                                    onTriggered: {
                                        back_options_menu.uncheckAll()
                                    }
                                }
                            }

                            MouseArea {
                                acceptedButtons: Qt.RightButton
                                onClicked: back_options_menu.popup()
                                anchors.fill: parent
                            }
                            delegate: Item {
                                visible: type !== "";
                                width: parent.width - 3
                                height: type === "" ? 0 : 26
                                Rectangle {
                                    anchors.fill: parent
                                    color: { switch(type) {
                                        case "bin": return "red";
                                        case "data": return "purple";
                                        case "system": return "steelblue";
                                        default: return "transparent";
                                        }
                                    }
                                    opacity: 0.2
                                }
                                CheckBox {
                                    id: backDelegateBox
                                    text: friendlyName + " (" + type + ") " + version
                                    width: Math.min(implicitWidth, parent.width - sizeText.width)
                                    clip: true
                                    checked: isMarked

                                    onCheckedChanged: isMarked = checked
                                    onClicked: {
                                        if (checked)
                                            appData.selectedSize += size;
                                        else
                                            appData.selectedSize -= size;
                                    }
                                    Connections {
                                        target: back_options_menu
                                        onCheckAll: {
                                            if (!backDelegateBox.checked) {
                                                backDelegateBox.checked = true;
                                                appData.selectedSize += size;
                                            }
                                        }
                                        onUncheckAll: {
                                            if (backDelegateBox.checked) {
                                                backDelegateBox.checked = false;
                                                appData.selectedSize -= size;
                                            }
                                        }
                                    }
                                }
                                Label {
                                    id: sizeText
                                    anchors.right: parent.right
                                    text: qsTr("%1 MB").arg((size / 1024 / 1024).toFixed(1)) + translator.lang
                                    font.pointSize: 12;
                                }
                            }
                        }
                    }
                }
            }

            Text {
                visible: i.backMethods
                id: totalText
                property double totalVal: 0.0
                text: (qsTr("Total:") + " " + (totalVal < 0 ? qsTr("Unknown Size") : qsTr("%1 MB").arg((totalVal / 1024 / 1024).toFixed(1)))) + translator.lang
                font.pointSize: 12
            }
        }

        Button {
            visible: /*!i.backMethods &&*/ !attemptLookup.running
            enabled: !i.installing && !i.backing && !i.restoring && i.device !== null && i.device.bbid !== ""
            text:  qsTr("Refresh Backup Sizes") + translator.lang
            onClicked: { appDataSelect.hide(); totalText.totalVal = 0; i.backupQuery(); attemptLookup.start(); }
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
