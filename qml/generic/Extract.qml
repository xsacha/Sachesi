import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.1
import "UI" 1.0

Item {
    Rectangle {
        visible: p.splitting
        anchors {bottom: parent.bottom; bottomMargin: 10; horizontalCenter: parent.horizontalCenter }
        height: 80; width: parent.width - 40; radius: 8
        z: 5;
        color: "gray"
        opacity: 0.95
        Column {
            spacing: 5
            anchors {top: parent.top; topMargin: 10; left: parent.left; leftMargin: parent.width * 0.5 - splitText.typeText.length * 9 }
            Row {
                spacing: 10

                Text {
                    id: splitText
                    property string typeText: "";
                    property int splitType: p.splitting
                    onSplitTypeChanged: {
                        switch(splitType) {
                        case 1: typeText = "Splitting Autoloader "; break;
                        case 2: typeText = "Combining Autoloader "; break;
                        case 3: typeText = "Extracting Image "; break;
                        case 4: typeText = "Extracting Apps "; break;
                        case 5: typeText = "Fetching required files "; break;
                        default: typeText = "Waiting "; break;
                        }
                    }

                    text: typeText + ((p.splitting == 5) ? "" : "(" + p.splitProgress + "%)");
                    font.pointSize: 14
                }
                BusyIndicator {
                    width: parent.height
                    height: parent.height
                }
            }
            Text {
                visible: p.splitProgress > 100
                text: "Percentages are not entirely accurate for QNX6 files."
                font.pointSize: 10
            }
        }

        Button {
            z: 6;
            visible: p.splitting == 2
            anchors {bottom: parent.bottom; bottomMargin: 10; horizontalCenter: parent.horizontalCenter}
            text: "Cancel";
            onClicked: p.abortSplit();
        }
    }

    ColumnLayout {
        anchors { fill: parent; margins: 15 }
        Layout.fillHeight: true
        ColumnLayout {
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
            Text {
                text: "Autoloader Tools"
                font.pointSize: 14
                font.bold: true
            }
            RowLayout {
                FileDialog {
                    id: split_files
                    title: "Extract Signed"
                    folder: settings.installFolder
                    onAccepted: {
                        p.splitAutoloader(fileUrl, osSelect.checked * 1 + radioSelect.checked * 2 + pinSelect.checked * 4);
                        settings.installFolder = folder;
                    }

                    nameFilters: [ "Signed Containers (*.exe *.bar *.zip)" ]
                }

                Button {
                    text: "Extract Signed";
                    enabled: !p.splitting
                    onClicked: split_files.open()
                }
                CheckBox {
                    id: osSelect
                    text: "OS"
                    checked: true
                }
                CheckBox {
                    id: radioSelect
                    text: "Radio"
                    checked: true
                }
                CheckBox {
                    visible: settings.advanced
                    id: pinSelect
                    text: "PINList"
                }
            }
            Label {
                text: "Split signed images from autoloader .exe, .bar or .zip"
                font.bold: true;
            }
            RowLayout {
                FileDialog {
                    id: combine_files
                    title: "Create Autoloader"
                    folder: settings.installFolder
                    onAccepted: {
                        p.combineAutoloader(fileUrls);
                        settings.installFolder = folder;
                    }

                    nameFilters: [ "Signed Images (*.signed)" ]
                }
                Button {
                    text: "Create from Folder"
                    enabled: !p.splitting
                    onClicked: {
                        combine_files.selectFolder = true
                        combine_files.open()
                    }
                }
                Button {
                    text: "Create from Files";
                    enabled: !p.splitting
                    onClicked: {
                        combine_files.selectFolder = false
                        combine_files.selectMultiple = true
                        combine_files.open()
                    }
                }
            }
            Label {
                text: "Create Autoloader .exe from .signed images"
                font.bold: true;
            }
        }
        // Implicit spacing by breaking layout
        ColumnLayout {
            Text {
                text: "Extraction Tools"
                font.pointSize: 14
                font.bold: true
            }
            // Dump Contents
            ColumnLayout {
                visible: settings.advanced
                RowLayout {
                    property int partValue: corePart.checked * 1 + userPart.checked * 2 + bootPart.checked * 4
                    Button {
                        text: "Dump Contents"
                        enabled: !p.splitting && parent.partValue
                        onClicked: if (!p.splitting) p.extractImage(0, parent.partValue);
                    }
                    CheckBox {
                        id: corePart
                        checked: true
                        text: "Core"
                    }
                    CheckBox {
                        id: userPart
                        checked: true
                        text: "User"
                    }
                    CheckBox {
                        id: bootPart
                        checked: false
                        text: "Boot"
                    }
                }
                Label {
                    text: "Dump all file contents"
                    font.bold: true;
                }
            }
            // Extract Image
            ColumnLayout {
                visible: settings.advanced
                RowLayout {
                    property int imageValue: rcfsImage.checked * 1 + qnxImage.checked * 2 + bootImage.checked * 4
                    Button {
                        text: "Extract Image"
                        enabled: !p.splitting && parent.imageValue
                        onClicked: if (!p.splitting) p.extractImage(1, parent.imageValue);
                    }
                    CheckBox {
                        id: rcfsImage
                        checked: true
                        text: "RCFS"
                    }
                    CheckBox {
                        id: qnxImage
                        text: "QNX6"
                    }
                    CheckBox {
                        id: bootImage
                        text: "IFS"
                    }
                }
                Label {
                    text: "Extracts filesystem image"
                    font.bold: true;
                }
            }
            ColumnLayout {
                Button {
                    text: "Extract Apps"
                    enabled: !p.splitting
                    onClicked: if (!p.splitting) p.extractImage(2, 2);
                }
                Label {
                    text: "Extract all bar archives"
                    font.bold: true;
                }
            }
        }
    }
}
