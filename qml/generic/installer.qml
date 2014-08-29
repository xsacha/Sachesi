import QtQuick 2.2
import QtQuick.Controls 1.2
import Drop 1.0
import AppLibrary 1.0
import "UI" 1.0

PageTab {
    id:main
    property string newLine: i.newLine
    property string details: ""
    property string lasterror: "\n"
    property bool connected: i.knownBattery > -1
    onNewLineChanged: details += i.newLine

    Rectangle {
        visible: i.dgProgress >= 0
        anchors {bottom: parent.bottom; bottomMargin: 10; horizontalCenter: parent.horizontalCenter }
        height: 100 + config.defaultSubtextSize; width: parent.width - 20; radius: 8
        z: 5
        color: "gray"
        opacity: 0.95
        Text {
            text: i.firmwareUpdate ? "Firmware Update" : "Install"
            font.pointSize: 14
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Rectangle {
            anchors {left: parent.left; leftMargin: 10; verticalCenter: parent.verticalCenter }
            color: "transparent"
            width: parent.width - 20; height: 50
            border.width: 1;
            Rectangle {
                x: 1; y: 1
                width: (i.dgProgress / 100) * parent.width - 2
                height: 50 - 2
                color: config.darkColor
            }
            Text {
                text: i.currentInstallName
                anchors {top: parent.top; topMargin: 2; horizontalCenter: parent.horizontalCenter }
                font.pixelSize: config.defaultSubtextSize
            }
            Text {
                anchors {bottom: parent.bottom; bottomMargin: 2; horizontalCenter: parent.horizontalCenter }
                text: "("+i.dgProgress+"%)"
                font.pixelSize: config.defaultSubtextSize
            }
        }
        Text {
            anchors {bottom: parent.bottom; bottomMargin: 4; horizontalCenter: parent.horizontalCenter }
            text: "Look at your device for total install percent."
            font.pixelSize: config.defaultSubtextSize
        }
        MouseArea {
            anchors.fill: parent
        }
    }

    Text {
        anchors { bottom: parent.bottom; left: parent.left; leftMargin: 20; bottomMargin: 12 }
        text: "<b>[</b>USB" + " ("+i.knownBattery+"%)<b>]</b>  " + "  <b>[</b>OS" + i.knownOS + "<b>]</b>" + (parent.width > 700 ? (" <b>[</b>" + i.knownName + "<b>]</b>") : "");
        font.pixelSize: config.defaultSubtextSize
    }

    Text {
        anchors { bottom: parent.bottom; right: parent.right; rightMargin: 20; bottomMargin: 12 }
        z: 2
        text: "<b>[</b>"+i.knownHW+"<b>]</b>"
        font.pixelSize: config.defaultSubtextSize
    }

    DropArea {
        anchors.fill: parent
        onFileDrop: {
            if (i.installing) { details += "Error: Your device can only process one task at a time. Please wait for previous install to complete.<br>" }
            else if (i.backing || i.restoring) { details += "Error: Your device can only process one task at a time. Please wait for backup process to complete<br>" }
            else { i.install(text); }
            tabs.currentIndex = 1
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
                font.pixelSize: config.defaultFontSize
            }
            RoundButton {
                id: install_folder
                text: "Folder"
                onClicked: {
                    if (i.installing) { details += "Error: Your device can only process one task at a time. Please wait for previous install to complete.<br>"; tabs.currentIndex = 1; }
                    else if (i.backing || i.restoring) { details += "Error: Your device can only process one task at a time. Please wait for backup process to complete.<br>"; tabs.currentIndex = 1; }
                    else { if(i.selectInstallFolder()) tabs.currentIndex = 1; }
                }
            }
            RoundButton {
                id: install_files
                text: ".bar(s)"
                onClicked: {
                    if (i.installing) { details += "Error: Your device can only process one task at a time. Please wait for previous install to complete.<br>"; tabs.currentIndex = 1; }
                    else if (i.backing || i.restoring) { details += "Error: Your device can only process one task at a time. Please wait for backup process to complete.<br>"; tabs.currentIndex = 1; }
                    else { if (i.selectInstall()) tabs.currentIndex = 1; }
                }
            }
        }
        Text {
            id: helpText
            text: "To install <b>.bar</b> files such as applications or firmware, you can just <b>Drag and Drop</b>."
            font.pixelSize: config.defaultSubtextSize
        }
        Row {
            visible: p.advanced
            spacing: 15
            RoundButton {
                id: wipe
                text: "Wipe"
                onClicked: i.wipe();
            }
            RoundButton {
                id: factorywipe
                text: "Factory"
                onClicked: i.factorywipe();
            }
            RoundButton {
                id: reboot
                text: "Reboot"
                onClicked: i.reboot();
            }
        }
    }

    TabView {
        id: tabs
        anchors { top: toolsColumn.bottom; topMargin: 15; left: toolsColumn.left }
        height: parent.height - (p.advanced ? 150 : 115) /*parent.height - 100 - parent.height / 5*/; width: parent.width - 30; z: 2;
        RoundButton {
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
                font.pixelSize: config.notificationFontSize
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
                        width: parent.width
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
                        Text {
                            anchors.right: parent.right
                            text: version
                            font.pixelSize: config.defaultFontSize;
                            clip: true
                        }
                    }
                }
            }
        }
    }
}
