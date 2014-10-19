import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import QtQuick.Window 2.1

Window {
    title: qsTr("Sachesi") + " " + version + " - " + qsTr("Version Lookup")
    visible: false
    onVisibleChanged: if (visible) {
                          x = window.x + (window.width - width) / 2
                          y = window.y + (window.height - height) / 2
                      }
    height: 140
    width: 430
    ColumnLayout {
        height: parent.height
        width: parent.width
        anchors { left: parent.left; leftMargin: 10 }
        RowLayout {
            Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
            Row {
                spacing: 1
                SpinBox {
                    id: major
                    prefix: "10."
                    width: qt_new ? implicitWidth : implicitWidth + 25
                    value: 3
                    maximumValue: 255
                    onEditingFinished: relookup.clicked()
                }
                SpinBox {
                    id: minor
                    width: qt_new ? implicitWidth : implicitWidth + 25
                    value: 1
                    maximumValue: 255
                    onEditingFinished: relookup.clicked()
                }
                SpinBox {
                    id: build
                    width: qt_new ? implicitWidth : implicitWidth + 25
                    value: 821
                    maximumValue: 9999
                    stepSize: 3
                    onEditingFinished: relookup.clicked()
                }
            }
            Button {
                id: relookup
                text:  qsTr("Lookup")
                enabled: !p.scanning
                onClicked: p.reverseLookup(device.selectedItem, variant.selectedItem, server.selectedItem, "10." + major.value + "." + minor.value + "." + build.value, skip_badlinks.checked, check_sdk.checked);
            }
            Button {
                property bool looking: false
                text: looking ? qsTr("Stop Scan") : qsTr("Autoscan")
                enabled: !p.scanning || looking
                onClicked: { looking = !looking; if (looking) { build.value += 3; relookup.clicked(); } }
                Timer {
                    id: autoLookup
                    interval: 10;
                    running: parent.looking && !p.scanning
                    onTriggered: {
                        if (p.scanning > 0)
                            return;
                        if (grabPotential.visible) {
                            parent.looking = false;
                        } else if (p.softwareRelease == "SR not in system") {
                            if (build.value >= 9998) {
                                minor.value++;
                                build.value = (build.value+3) % 10000;
                            } else
                                build.value += 3;
                            relookup.clicked();
                        }
                    }
                }
            }
        }
        RowLayout {
            CheckBox {
                id: skip_badlinks
                enabled: !check_sdk.checked
                text:  qsTr("Find next available links")
            }
            CheckBox {
                id: check_sdk
                text:  qsTr("Check for SDK")
                onCheckedChanged: {
                    if (checked) build.value--
                    else build.value++
                    relookup.clicked();
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignVCenter
            Text {
                Layout.alignment: Qt.AlignLeft
                text:  qsTr("Software Release: ") + p.softwareRelease
                font.pointSize: 12
            }
            RowLayout {
                Layout.alignment: Qt.AlignRight
                property string osVersion: ""
                visible: p.softwareRelease.charAt(0) == "1" || p.softwareRelease.charAt(0) == "2"
                onVisibleChanged: if (visible) osVersion = "10." + major.value + "." + minor.value + "." + build.value
                Button {
                    id: grabPotential
                    enabled: p.hasPotentialLinks // Exists?
                    text: enabled ? qsTr("Grab Public Links") : qsTr("No Links Available")
                    onClicked: p.grabPotentialLinks(p.softwareRelease, parent.osVersion, check_sdk.checked)
                }
            }
        }
        Button {
            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
            text:  qsTr("Hide")
            onClicked: close();
        }
    }
}
