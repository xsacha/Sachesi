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
            anchors {top: parent.top; topMargin: 10; left: parent.left; leftMargin: parent.width * 0.5 - splitText.splitType.length * 9 }
            Row {
                spacing: 10

                Text {
                    id: splitText
                    property string splitType: "";
                    property int splitting: p.splitting
                    onSplittingChanged: {
                        switch(splitting) {
                        case 1: splitType = "Splitting Autoloader "; break;
                        case 2: splitType = "Combining Autoloader "; break;
                        case 3: splitType = "Extracting Image "; break;
                        case 4: splitType = "Extracting Apps "; break;
                        case 5: splitType = "Fetching required files "; break;
                        default: splitType = "Waiting "; break;
                        }
                    }

                    text: splitType + ((p.splitting == 5) ? "" : "(" + p.splitProgress + "%)");
                    font.pointSize: 14
                }
                BusyIndicator {
                    width: parent.height
                    height: parent.height
                }
            }
            Text {
                visible: p.splitProgress > 100
                text:  qsTr("Percentages are not entirely accurate for QNX6 files.")
                font.pointSize: 10
            }
        }

        Button {
            z: 6;
            visible: p.splitting == 2
            anchors {bottom: parent.bottom; bottomMargin: 10; horizontalCenter: parent.horizontalCenter}
            text:  qsTr("Cancel");
            onClicked: p.abortSplit();
        }
    }
    ColumnLayout {
        anchors { fill: parent; margins: 15 }
        Layout.fillHeight: true
        Text {
            text:  qsTr("Extraction Tools")
            font.pointSize: 14
            font.bold: true
        }
        // Extract Signed
        ColumnLayout {
            RowLayout {
                FileDialog {
                    id: split_files
                    title:  qsTr("Extract Signed")
                    folder: settings.installFolder
                    onAccepted: {
                        p.splitAutoloader(fileUrl, userSelect.checked * 1 + osSelect.checked * 2 + radioSelect.checked * 4 + ifsSelect.checked * 8 + pinSelect.checked * 16);
                        settings.installFolder = folder;
                    }

                    nameFilters: [ "Signed Containers (*.exe *.bar *.zip)" ]
                }

                Button {
                    text:  qsTr("Extract Signed");
                    enabled: !p.splitting
                    onClicked: split_files.open()
                }
                CheckBox {
                    id: userSelect
                    text:  qsTr("User")
                    checked: true
                }
                CheckBox {
                    id: osSelect
                    text:  qsTr("OS")
                    checked: true
                }
                CheckBox {
                    id: radioSelect
                    text:  qsTr("Radio")
                    checked: true
                }
                CheckBox {
                    id: ifsSelect
                    text:  "IFS"
                    checked: true
                }
                CheckBox {
                    visible: settings.advanced
                    id: pinSelect
                    text:  qsTr("PINList")
                }
            }
            Label {
                text:  qsTr("Split signed images from autoloader .exe, .bar or .zip")
                font.bold: true;
            }
        }
        // Extract Image
        ColumnLayout {
            visible: settings.advanced
            RowLayout {
                property int imageValue: rcfsImage.checked * 1 + qnxImage.checked * 2 + bootImage.checked * 4
                Button {
                    text:  qsTr("Extract Image")
                    enabled: !p.splitting && parent.imageValue
                    onClicked: if (!p.splitting) p.extractImage(1, parent.imageValue);
                }
                CheckBox {
                    id: rcfsImage
                    checked: true
                    text:  "RCFS"
                }
                CheckBox {
                    id: qnxImage
                    text:  "QNX6"
                }
                CheckBox {
                    id: bootImage
                    checked: true
                    text:  "IFS"
                }
            }
            Label {
                text:  qsTr("Extracts filesystem image")
                font.bold: true;
            }
        }
        ColumnLayout {
            Button {
                text:  qsTr("Extract Apps")
                enabled: !p.splitting
                onClicked: if (!p.splitting) p.extractImage(2, 2);
            }
            Label {
                text:  qsTr("Extracts all bar archives from a debrick/repair image")
                font.bold: true;
            }
        }
        ColumnLayout {
            RowLayout {
                FileDialog {
                    id: combine_files
                    title:  qsTr("Create Autoloader")
                    folder: settings.installFolder
                    onAccepted: {
                        p.combineAutoloader(fileUrls);
                        settings.installFolder = folder;
                    }

                    nameFilters: [ "Signed Images (*.signed)" ]
                }
                Button {
                    text:  qsTr("Create from Folder")
                    enabled: !p.splitting
                    onClicked: {
                        combine_files.selectFolder = true
                        combine_files.open()
                    }
                }
                Button {
                    text:  qsTr("Create from Files");
                    enabled: !p.splitting
                    onClicked: {
                        combine_files.selectFolder = false
                        combine_files.selectMultiple = true
                        combine_files.open()
                    }
                }
            }
            Label {
                text:  qsTr("Create Autoloader .exe from .signed images")
                font.bold: true;
            }
        }
        // Dump Contents
        ColumnLayout {
            visible: settings.advanced
            RowLayout {
                property int partValue: corePart.checked * 1 + userPart.checked * 2 + bootPart.checked * 4
                Button {
                    text:  qsTr("Dump Contents")
                    enabled: !p.splitting && parent.partValue
                    onClicked: if (!p.splitting) p.extractImage(0, parent.partValue);
                }
                CheckBox {
                    id: corePart
                    checked: true
                    text:  qsTr("Core")
                }
                CheckBox {
                    id: userPart
                    checked: true
                    text:  qsTr("User")
                }
                CheckBox {
                    id: bootPart
                    checked: false
                    text:  qsTr("Boot")
                }
            }
            Label {
                text:  qsTr("Dump all file contents")
                font.bold: true;
            }
        }
    }
}
