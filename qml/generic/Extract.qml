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
                    property int splitting: p.splitting + translator.lang
                    onSplittingChanged: {
                        switch(splitting) {
                        case 1: splitType = qsTr("Splitting Autoloader"); break;
                        case 2: splitType = qsTr("Combining Autoloader"); break;
                        case 3: splitType = qsTr("Extracting Image"); break;
                        case 4: splitType = qsTr("Extracting Apps"); break;
                        case 5: splitType = qsTr("Fetching required files"); break;
                        default: splitType = qsTr("Waiting"); break;
                        }
                    }

                    text: splitType + ((p.splitting == 5) ? "" : " (" + p.splitProgress + "%)");
                    font.pointSize: 14
                }
                BusyIndicator {
                    width: parent.height
                    height: parent.height
                }
            }
            Text {
                visible: p.splitProgress > 100
                text: qsTr("Percentages are not entirely accurate for QNX6 files.") + translator.lang
                font.pointSize: 10
            }
        }

        Button {
            z: 6;
            visible: p.splitting == 2
            anchors {bottom: parent.bottom; bottomMargin: 10; horizontalCenter: parent.horizontalCenter}
            text: qsTr("Cancel") + translator.lang
            onClicked: p.abortSplit();
        }
    }
    ColumnLayout {
        anchors { fill: parent; margins: 15 }
        Layout.fillHeight: true
        Text {
            text: qsTr("Extraction Tools") + translator.lang
            font.pointSize: 14
            font.bold: true
        }
        ColumnLayout {
            RowLayout {
                FileDialog {
                    id: combine_files
                    title:  qsTr("Create Autoloader") + translator.lang
                    folder: settings.installFolder
                    onAccepted: {
                        p.combineAutoloader(fileUrls);
                        settings.installFolder = folder;
                    }

                    nameFilters: [ qsTr("Signed Images") + " (*.signed)" + translator.lang ]
                }
                Button {
                    text:  qsTr("Create from Folder") + translator.lang
                    enabled: !p.splitting
                    onClicked: {
                        combine_files.selectFolder = true
                        combine_files.open()
                    }
                }
                Button {
                    text:  qsTr("Create from Files") + translator.lang
                    enabled: !p.splitting
                    onClicked: {
                        combine_files.selectFolder = false
                        combine_files.selectMultiple = true
                        combine_files.open()
                    }
                }
            }
            Label {
                text:  qsTr("Create Autoloader .exe from .signed images") + translator.lang
                font.bold: true;
            }
        }
        // Extract Signed
        ColumnLayout {
            RowLayout {
                FileDialog {
                    id: split_files
                    title:  qsTr("Extract Signed") + translator.lang
                    folder: settings.installFolder
                    onAccepted: {
                        p.splitAutoloader(fileUrl, userSelect.checked * 1 + osSelect.checked * 2 + radioSelect.checked * 4 + ifsSelect.checked * 8 + pinSelect.checked * 16);
                        settings.installFolder = folder;
                    }

                    nameFilters: [ qsTr("Signed Containers") + " (*.exe *.bar *.zip)" + translator.lang ]
                }

                Button {
                    text:  qsTr("Extract Signed") + translator.lang
                    enabled: !p.splitting
                    onClicked: split_files.open()
                }
                CheckBox {
                    visible: settings.advanced
                    id: userSelect
                    text:  qsTr("User") + translator.lang
                    checked: true
                }
                CheckBox {
                    visible: settings.advanced
                    id: osSelect
                    text:  qsTr("OS") + translator.lang
                    checked: true
                }
                CheckBox {
                    visible: settings.advanced
                    id: radioSelect
                    text:  qsTr("Radio") + translator.lang
                    checked: true
                }
                CheckBox {
                    visible: settings.advanced
                    id: ifsSelect
                    text:  "IFS"
                    checked: true
                }
                CheckBox {
                    visible: settings.advanced
                    id: pinSelect
                    text:  qsTr("PINList") + translator.lang
                }
            }
            Label {
                text:  qsTr("Split .signed from autoloader .exe, .bar or .zip") + translator.lang
                font.bold: true;
            }
        }
        // Extract Apps
        ColumnLayout {
            Button {
                text:  qsTr("Extract Apps") + translator.lang
                enabled: !p.splitting
                onClicked: if (!p.splitting) p.extractImage(2, 2);
            }
            Label {
                text:  qsTr("Extracts all bar archives from a debrick/repair .signed") + translator.lang
                font.bold: true;
            }
            Label {
                text: qsTr("Note: To extract apps from a .bar, please split it first (above)") + translator.lang
            }
        }
        // Extract Image
        ColumnLayout {
            visible: settings.advanced
            RowLayout {
                property int imageValue: rcfsImage.checked * 1 + qnxImage.checked * 2 + bootImage.checked * 4
                Button {
                    text:  qsTr("Extract Image") + translator.lang
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
                text:  qsTr("Extracts filesystem image") + translator.lang
                font.bold: true;
            }
        }
        // Dump Contents
        ColumnLayout {
            visible: settings.advanced
            RowLayout {
                property int partValue: corePart.checked * 1 + userPart.checked * 2 + bootPart.checked * 4
                Button {
                    text:  qsTr("Dump Contents") + translator.lang
                    enabled: !p.splitting && parent.partValue
                    onClicked: if (!p.splitting) p.extractImage(0, parent.partValue);
                }
                CheckBox {
                    id: corePart
                    checked: true
                    text:  qsTr("Core") + translator.lang
                }
                CheckBox {
                    id: userPart
                    checked: true
                    text:  qsTr("User") + translator.lang
                }
                CheckBox {
                    id: bootPart
                    checked: false
                    text:  qsTr("Boot") + translator.lang
                }
            }
            Label {
                text:  qsTr("Dump all file contents") + translator.lang
                font.bold: true;
            }
        }
    }
}
