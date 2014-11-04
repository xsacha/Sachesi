import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import QtQuick.Window 2.1

Window {
    title: "Sachesi " + version + " – " + qsTr("Version Lookup") + translator.lang
    visible: false
    onVisibleChanged: if (visible) {
                          x = window.x + (window.width - width) / 2
                          y = window.y + (window.height - height) / 2
                      }
    height: 290
    width: 490
    ColumnLayout {
        height: parent.height
        width: parent.width
        anchors { left: parent.left; leftMargin: 10 }
        RowLayout {
        GroupBox {
            title: qsTr("Stop on:") + translator.lang
            Column {
                ExclusiveGroup {
                    id: group
                    onCurrentChanged: scanner.findExisting = current.item
                }
                RadioButton {
                    property int item: 0
                    text: qsTr("Next Found") + translator.lang
                    exclusiveGroup: group
                    checked: true
                }
                RadioButton {
                    property int item: 1
                    text: qsTr("Next Available Links") + translator.lang
                    exclusiveGroup: group
                }
                RadioButton {
                    property int item: 2
                    text: qsTr("Never") + translator.lang
                    exclusiveGroup: group
                }
                Button {
                    text: scanner.isAuto ? qsTr("Stop Scan") : qsTr("Autoscan") + translator.lang
                    onClicked: {
                        scanner.isAuto = !scanner.isAuto;
                        if (scanner.isAuto) { build.value += 3; relookup.clicked(); }
                    }
                    property bool finished: scanner.finishedScan
                    onFinishedChanged: {
                        if (finished && !scanner.isActive && scanner.isAuto) {
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
        ColumnLayout {
            Layout.alignment: Qt.AlignVCenter
            visible: scanner.curRelease !== null && scanner.curRelease.srVersion !== ""
            Text {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("SR: %1").arg(scanner.curRelease !== null ? scanner.curRelease.srVersion : "") + " | " + qsTr("OS: %1").arg(scanner.curRelease !== null ? scanner.curRelease.osVersion : "") + translator.lang
                font.pointSize: 12
            }
            Label {
                Layout.alignment: Qt.AlignHCenter
                text: {
                    var ret = ""
                    if (scanner.curRelease !== null) {
                        if (scanner.curRelease.activeServers & 1)
                            ret += qsTr("Production") + " "
                        if (scanner.curRelease.activeServers & 2)
                            ret += qsTr("Beta") + " "
                        if (scanner.curRelease.activeServers & 4)
                            ret += qsTr("Alpha") + " "
                        if (ret.length > 0)
                            ret = qsTr("Servers:") + " " + ret
                    }
                    return ret + translator.lang;
                }
            }
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                property string osVersion: ""
                visible: scanner.curRelease !== null && scanner.curRelease.srVersion != ""
                Button {
                    id: grabPotential
                    enabled: scanner.curRelease !== null && scanner.curRelease.baseUrl !== ""
                    text: enabled ? qsTr("Grab Public Links") : qsTr("No Links Available") + translator.lang
                    onClicked: scanner.generatePotentialLinks()
                }
            }
        }
        }
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
                }
                SpinBox {
                    id: minor
                    width: qt_new ? implicitWidth : implicitWidth + 25
                    value: 1
                    maximumValue: 255
                }
                SpinBox {
                    id: build
                    width: qt_new ? implicitWidth : implicitWidth + 25
                    value: 938
                    maximumValue: 9999
                    stepSize: 3
                }
            }
            Button {
                id: relookup
                text: qsTr("Lookup") + translator.lang
                enabled: !scanner.isActive && !scanner.isAuto
                onClicked: scanner.reverseLookup("10." + major.value + "." + minor.value + "." + build.value);
            }
        }
        Button {
            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
            text: qsTr("Hide") + translator.lang
            onClicked: close();
        }
    }
}
