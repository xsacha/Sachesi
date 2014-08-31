import QtQuick 2.2
import QtQuick.Controls 1.2
import BackupTools 1.0
import "UI" 1.0

TabView {
    id: main
    USBConnect { anchors.fill: parent }
    Item {
        visible: i.knownBattery > -1
        anchors.fill: parent

        Timer {
            id: attemptLookup
            running: false
            interval: 12000
        }

        Column {
            anchors { top: parent.top; topMargin: 20; left: parent.left; leftMargin: 20 }
            spacing: 20
            Column {
                spacing: 10
                Text {
                    text: "Options"
                    font.pointSize: 14
                    font.bold: true
                }
                Column {
                    id: options
                    property int value: 0
                    spacing: 10
                    Row {
                        visible: !i.backMethods && attemptLookup.running
                        spacing: 10
                        Text {
                            text: "Loading Backup Options"
                            font.pointSize: 12
                        }
                        BusyIndicator {
                            width: parent.height
                            height: parent.height
                        }
                    }

                    Column {
                        Repeater {
                            model: i.backMethods
                            delegate: Row {
                                CheckBox {
                                    text: i.backNames[index] + " (" + i.backSizes[index] + " MB)" // index
                                    onCheckedChanged: {
                                        if (checked) options.value += 1 << index; else options.value -= 1 << index;
                                        if (checked) totalText.totalVal += parseInt(i.backSizes[index],10); else totalText.totalVal -= i.backSizes[index];
                                    }
                                }
                            }
                        }
                    }
                    Text {
                        visible: i.backMethods
                        id: totalText
                        property int totalVal: 0
                        text: "Total: " + totalVal + " MB"
                        font.pointSize: 12
                    }
                }
            }

            Button {
                visible: !i.backMethods && !attemptLookup.running
                text: "Load Backup Options"
                onClicked: { totalText.totalVal = 0; attemptLookup.start(); i.backupQuery() }
            }

            Row {
                visible: i.backMethods
                spacing: 20
                Button {
                    text: "Create backup"
                    enabled: !i.installing && !i.backing && !i.restoring && options.value != 0
                    onClicked: i.selectBackup(options.value)
                }
                Button {
                    text: "Restore backup"
                    enabled: !i.installing && !i.backing && !i.restoring && options.value != 0
                    onClicked: i.selectRestore(options.value)
                }
            }
        }
        Rectangle {
            id: progressBar
            visible: i.backing || i.restoring
            anchors {bottom: parent.bottom; bottomMargin: 20; horizontalCenter: parent.horizontalCenter }
            height: 40 + config.notificationFontSize; width: parent.width - parent.width / 4; radius: 8
            z: 5;
            color: "gray"
            opacity: 0.95
            Column {
                anchors {verticalCenter: parent.verticalCenter; left: parent.left; leftMargin: parent.width / 2 - 150 }
                Text {
                    font.pointSize: 12
                    text: (i.backing ? "Creating" : "Restoring") + " Backup (" + i.backProgress + "%)";
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
}
