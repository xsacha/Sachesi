import QtQuick 2.2
import QtQuick.Controls 1.1
import AppLibrary 1.0
import "UI" 1.0

TabView {
    id: main
    USBConnect { anchors.fill: parent }
    property string newLine: i.newLine
    property string details: ""
    property string lasterror: "\n"
    onNewLineChanged: details += i.newLine

    Item {
        visible: i.knownBattery > -1
        anchors.fill: parent

        Rectangle {
            visible: i.dgProgress >= 0
            anchors {bottom: parent.bottom; bottomMargin: 10; horizontalCenter: parent.horizontalCenter }
            width: parent.width / 3; height: Math.min(parent.height / 2, width + 20); radius: 8
            z: 5
            color: "lightgray"
            opacity: 0.6
            Text {
                id: titleText
                text: i.firmwareUpdate ? "Firmware Update" : "Install"
                font.pointSize: 14
                anchors.horizontalCenter: parent.horizontalCenter
            }
            CircleProgress {
                width: parent.width - 10; height: Math.min(parent.height - 20, width);
                anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom }
                currentValue: i.dgProgress
                text: i.curInstallName
            }
        }

        DropArea {
            anchors.fill: parent
            onDropped: {
                if (drop.hasUrls) {
                    if (i.installing) {
                        details += "Error: Your device can only process one task at a time. Please wait for previous install to complete.<br>"
                    } else if (i.backing || i.restoring) {
                        details += "Error: Your device can only process one task at a time. Please wait for backup process to complete<br>"
                    } else {
                        var fileList = []
                        for (var url in drop.urls)
                            fileList[url] = drop.urls[url]
                        i.install(fileList);
                    }
                    tabs.currentIndex = 1
                }
            }
        }
        Column {
            id: toolsColumn
            anchors {top: parent.top; topMargin: 20; left: parent.left; leftMargin: 20 }
            spacing: 10
            Row {
                spacing: 15
                Text {
                    text: "Install:"
                    font.pointSize: 12
                }
                Button {
                    id: install_folder
                    text: "Folder"
                    onClicked: {
                        if (i.installing) {
                            details += "Error: Your device can only process one task at a time. Please wait for previous install to complete.<br>";
                        } else if (i.backing || i.restoring) {
                            details += "Error: Your device can only process one task at a time. Please wait for backup process to complete.<br>";
                        } else {
                            i.selectInstallFolder()
                        }
                        tabs.currentIndex = 1;
                    }
                }
                Button {
                    id: install_files
                    text: ".bar(s)"
                    onClicked: {
                        if (i.installing) {
                            details += "Error: Your device can only process one task at a time. Please wait for previous install to complete.<br>"; tabs.currentIndex = 1;
                        } else if (i.backing || i.restoring) {
                            details += "Error: Your device can only process one task at a time. Please wait for backup process to complete.<br>"; tabs.currentIndex = 1;
                        } else {
                            i.selectInstall()
                        }
                        tabs.currentIndex = 1;
                    }
                }
            }
            Text {
                id: helpText
                text: "To install <b>.bar</b> files such as applications or firmware, you can just <b>Drag and Drop</b>."
                font.pointSize: 10
            }
            Row {
                visible: p.advanced
                spacing: 15
                Button {
                    id: wipe
                    text: "Wipe"
                    onClicked: i.wipe();
                }
                Button {
                    id: factorywipe
                    text: "Factory"
                    onClicked: i.factorywipe();
                }
                Button {
                    id: reboot
                    text: "Reboot"
                    onClicked: i.reboot();
                }
            }
        }

        TabView {
            id: tabs
            anchors { top: toolsColumn.bottom; topMargin: 15; left: toolsColumn.left }
            height: parent.height - (p.advanced ? 135 : 100); width: parent.width - 30; z: 2;
            Button {
                anchors { top: parent.top; topMargin:-height; right: parent.right }
                id: list_files
                text: "Refresh"
                onClicked: i.scanProps();
            }
            Component.onCompleted: { addTab("Apps", app_tab); addTab("Log", log_tab); }
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
                    visible: appView.count > 0
                    id: delete_files
                    anchors {right: parent.left; rightMargin: -2; top: parent.top }
                    property bool uninstalling: false
                    enabled: !i.installing
                    onEnabledChanged: if (enabled && uninstalling) { uninstalling = false; }
                    source: "trash.png"
                    smooth: true
                    width: config.notificationFontSize; height: config.notificationFontSize
                    scale: delMouse.pressed ? 0.8 : 1.0
                    opacity: uninstalling ? 0.6 : 1.0
                    Behavior on scale { NumberAnimation { duration: 100 } }
                    BusyIndicator {
                        id: delCircle
                        visible: delete_files.uninstalling
                        anchors.fill: parent
                    }
                    MouseArea {
                        id: delMouse
                        anchors.fill: parent
                        onClicked: { if (i.uninstallMarked()) delete_files.uninstalling = true; }
                    }
                }
                Image {
                    visible: appView.count > 0
                    id: export_files
                    anchors {right: parent.left; rightMargin: -1; bottom: parent.bottom }
                    source: "text.png"
                    smooth: true
                    width: config.notificationFontSize - 2; height: config.notificationFontSize - 2
                    scale: exportMouse.pressed ? 0.8 : 1.0
                    MouseArea {
                        id: exportMouse
                        anchors.fill: parent
                        onClicked: { i.exportInstalled(); }
                    }
                }
                Text {
                    visible: appView.count == 0
                    anchors.centerIn: parent
                    font.pointSize: 14
                    text: "Use 'Refresh' to update list"
                }
                ScrollView {
                    anchors.fill: parent
                    ListView {
                        id: appView
                        anchors.fill: parent
                        spacing: 3
                        clip: true
                        model: i.appList
                        delegate: Item {
                            visible: type !== "";
                            width: parent.width - 3
                            height: type === "" ? 0 : config.notificationFontSize
                            Rectangle {
                                anchors.fill: parent
                                color: { switch(type) {
                                    case "os": return "red";
                                    case "radio": return "maroon";
                                    case "application": if (friendlyName.indexOf("sys.data") === 0) return "purple"; else  return "steelblue";
                                    default: return "transparent";
                                    }
                                }
                                opacity: 0.2
                            }
                            CheckBox {
                                text: friendlyName
                                checked: isMarked
                                onCheckedChanged: isMarked = checked;
                            }
                            Label {
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
}
