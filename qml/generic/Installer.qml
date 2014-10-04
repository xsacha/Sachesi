import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0
import AppLibrary 1.0
import "UI" 1.0

Item {
    id: main
    property string newLine: i.newLine
    property string details: ""
    property string lasterror: "\n"
    onNewLineChanged: details += i.newLine
    visible: i.knownBattery > -1
    anchors.fill: parent

    Button {
        visible: i.dgProgress >= 0 && !installWin.visible
        anchors {bottom: parent.bottom; bottomMargin: 10; horizontalCenter: parent.horizontalCenter }
        text:  qsTr("View Install (" + i.dgProgress + ")")
        onClicked: installWin.visible = true
    }

    Window {
        id: installWin
        visible: i.dgProgress >= 0
        width: parent.width / 3; height: Math.min(parent.height / 2, width + 20);
        onVisibleChanged: if (visible) {
                              x = window.x + (window.width - width) / 2
                              y = window.y + (window.height - height) / 2
                          }
        color: "lightgray"
        title: qsTr(i.firmwareUpdate ? "Firmware Update" : "Install")
        CircleProgress {
            anchors.fill: parent
            currentValue: i.curDGProgress
            overallValue: i.dgProgress
            curId: i.dgPos + 1
            maxId: i.dgMaxPos
            text: (i.curDGProgress != 100) ? ( i.curDGProgress < 50 ? "Sending " : "Installing ") : "Sent " + i.curInstallName
        }
    }

    DropArea {
        anchors.fill: parent
        onDropped: {
            if (drop.hasUrls) {
                i.install(drop.urls);
                tabs.currentIndex = 1
            }
        }
    }
    ColumnLayout {
        anchors {fill: parent; margins: 15 }
        Label {
            Layout.fillWidth: true
            text:  qsTr("To install <b>.bar</b> files such as applications or firmware, you can just <b>Drag and Drop</b> to this page. Otherwise, select the options below:")
            wrapMode: Text.Wrap
            font.pointSize: 12
        }
        Row {
            spacing: 15
            FileDialog {
                id: install_files
                title: qsTr("Install applications to device")
                folder: settings.installFolder
                onAccepted: {
                    i.install(fileUrls)
                    tabs.currentIndex = 1
                    settings.installFolder = folder;
                }

                selectMultiple: true
                nameFilters: [ qsTr("Blackberry Installable (*.bar)") ]
            }
            Button {
                text:  qsTr("Install Folder")
                onClicked: {
                    if (i.installing)
                        details += qsTr("Error: Your device can only process one task at a time. Please wait for previous install to complete.<br>;")
                    else if (i.backing || i.restoring)
                        details += qsTr("Error: Your device can only process one task at a time. Please wait for backup/restore process to complete.<br>")
                    else {
                        install_files.title = qsTr("Select Folder")
                        install_files.selectFolder = true;
                        install_files.open();
                    }
                }
            }
            Button {
                text:  qsTr("Install Files")
                onClicked: {
                    if (i.installing)
                        details += qsTr("Error: Your device can only process one task at a time. Please wait for previous install to complete.<br>;")
                    else if (i.backing || i.restoring)
                        details += qsTr("Error: Your device can only process one task at a time. Please wait for backup/restore process to complete.<br>")
                    else {
                        install_files.title = qsTr("Select Files")
                        install_files.selectFolder = false;
                        install_files.selectMultiple = true;
                        install_files.open();
                    }
                }
            }
        }
        GroupBox {
            title:  qsTr("Advanced Tools")
            visible: settings.advanced
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
        TabView {
            id: tabs
            Layout.alignment: Qt.AlignBottom
            Layout.fillHeight: true
            Layout.fillWidth: true
            Button {
                anchors { top: parent.top; topMargin:-height; right: parent.right }
                id: list_files
                text:  qsTr("Refresh")
                onClicked: i.scanProps();
            }
            Component.onCompleted: { addTab(qsTr("Your Applications"), app_tab); addTab(qsTr("Log"), log_tab); }
        }
    }

    // Log
    Component {
        id: log_tab
        TextArea {
            id: updateMessage
            width: tabs.width; height: tabs.height
            textFormat: TextEdit.RichText
            selectByKeyboard: true
            wrapMode: TextEdit.WrapAnywhere
            readOnly: true
            text: details
        }
    }

    // Applications
    Component {
        id: app_tab
        Item {
            Image {
                id: uninstall_notifier
                visible: uninstalling
                property bool uninstalling: false
                anchors {centerIn: parent; verticalCenterOffset: -parent.height / 4}
                source: "trash.png"
                width: 75; height: 75
                opacity: 0.8
                BusyIndicator {
                    anchors.fill: parent
                }
            }
            Text {
                visible: appView.count == 0
                anchors.centerIn: parent
                font.pointSize: 14
                text:  qsTr("Use 'Refresh' to update list")
            }
            ScrollView {
                anchors.fill: parent
                ListView {
                    id: appView
                    anchors.fill: parent
                    spacing: 3
                    clip: true
                    model: i.appList
                    Menu {
                        id: apps_menu
                        visible: appView.count > 0
                        title:  qsTr("Options")
                        MenuItem {
                            text:  qsTr("Uninstall Marked")
                            iconSource: "trash.png"
                            enabled: !i.installing
                            onEnabledChanged: if (enabled && uninstall_notifier.uninstalling) { uninstall_notifier.uninstalling = false; }
                            onTriggered: { if (i.uninstallMarked()) uninstall_notifier.uninstalling = true; }
                        }
                        MenuItem {
                            text:  qsTr("Show Installed Apps")
                            iconSource: "text.png"
                            onTriggered: i.exportInstalled();
                        }
                    }

                    MouseArea {
                        enabled: appView.count > 0
                        acceptedButtons: Qt.RightButton
                        onClicked: apps_menu.popup()
                        anchors.fill: parent
                    }
                    delegate: Item {
                        visible: type !== "";
                        width: parent.width - 3
                        height: type === "" ? 0 : 26
                        Rectangle {
                            anchors.fill: parent
                            color: { switch(type) {
                                case "os": return "red";
                                case "radio": return "purple";
                                case "application": if (friendlyName.indexOf("sys.data") === 0) return "lightblue"; else  return "steelblue";
                                default: return "transparent";
                                }
                            }
                            opacity: 0.2
                        }
                        CheckBox {
                            text: friendlyName
                            width: Math.min(implicitWidth, parent.width - versionText.width)
                            clip: true
                            checked: isMarked
                            onCheckedChanged: isMarked = checked;
                        }
                        Label {
                            id: versionText
                            anchors.right: parent.right
                            text: version
                            font.pointSize: 12;
                        }
                    }
                }
            }
        }
    }
}
