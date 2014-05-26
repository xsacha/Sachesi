import QtQuick 1.1
import "mcc.js" as MCC

Rectangle {
    id: main
    width: 480
    height: 340
    radius: 8
    SystemPalette {id: pal}
    gradient: Gradient {
        GradientStop { position: 0.0; color: pal.mid }
        GradientStop { position: 1.0; color: pal.midlight }
    }

    border.width: 2
    border.color: pal.shadow
    property bool init: p.versionRelease === ""
    state: "initing"

    Rectangle {
        visible: p.downloading
        anchors {bottom: parent.bottom; bottomMargin: 10; horizontalCenter: parent.horizontalCenter }
        height: 110; width: parent.width - 20; radius: 8
        z: 5
        color: "gray"
        opacity: 0.95
        Text {
            text: "Download"
            font.pointSize: 14
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Text {
            id: dlText
            anchors {top: parent.top; topMargin: 40; left: parent.left; leftMargin: 10 }
            property int thisId: p.currentId + 1
            text: "Downloading (" + thisId + " of " + p.maxId + "): "
            font.pointSize: 12
        }
        Rectangle {
            anchors {left: dlText.right; leftMargin: 10; verticalCenter: dlText.verticalCenter }
            color: "transparent"
            width: 240; height: 30
            border.width: 1;
            Rectangle {
                x: 1; y: 1
                width: (p.dlProgress / 100) * parent.width - 2
                height: 30 - 2
                color: pal.dark
            }
            Text {
                text: p.currentFile
                anchors {top: parent.top; topMargin: 2; horizontalCenter: parent.horizontalCenter }
            }
            Text {
                anchors {bottom: parent.bottom; bottomMargin: 2; horizontalCenter: parent.horizontalCenter }
                text: "("+p.dlProgress+"%)"
            }
        }
        RoundButton {
            anchors {bottom: parent.bottom; bottomMargin: 10; horizontalCenter: parent.horizontalCenter}
            text: "Cancel"
            mouse.onClicked: p.abortDL()
        }
        MouseArea {
            anchors.fill: parent
        }
    }
    RoundButton {
        id: scanButton
        anchors { bottom: parent.bottom; bottomMargin: 40 }
        anchors.leftMargin: 60
        enabled: !p.scanning
        text: p.scanning ? "Searching..." : "Search"
        mouse.onClicked: { p.updateDetailRequest(country.value, carrier.value, device.selectedItem, variant.selectedItem, mode.selectedItem, server.selectedItem/*, version.selectedItem*/) }
    }
    Column {
        spacing: 10
        id: urlLinks
        anchors { right: parent.right; rightMargin: 60; bottom: parent.bottom; bottomMargin: 40 }
        RoundButton {
            text: "Grab Links"
            mouse.onClicked: p.grabLinks()
        }
        RoundButton {
            text: "Download All"
            mouse.onClicked: {p.dlProgress = -1; p.downloadLinks() }
        }
    }

    Column {
        id: variables
        spacing: 4 + (parent.height - 260) / 7
        anchors.top: parent.top
        anchors.topMargin: 30
        TextCouple {
            id: country
            type: "Country"
            subtext: "Romania"
            value: "226"
            onValueChanged: if (value.length == 3) subtext = MCC.to_country(value);
            before: carrier.thisid
            after: carrier.thisid
        }
        TextCouple {
            id: carrier
            type: "Carrier"
            value: "10"
            before: country.thisid
            after: country.thisid
        }
        TextCoupleSelect {
            id: device
            z: 11
            type: "Device"
            initialText: "Z10"
            listModel: ListModel {
                ListElement { name: "Z10" }
                ListElement { name: "Q10" }
                ListElement { name: "Q5" }
                ListElement { name: "Dev Alpha" }
                ListElement { name: "Playbook" }
            }
            onExpanded: { mode.close(); variant.close(); }
            onSelectedItemChanged: {
                switch (selectedItem) {
                case 4:
                    variant.listModel = playbook
                    break;
                case 3:
                    variant.listModel = devalpha
                    break;
                case 2:
                    variant.listModel = r10;
                    break;
                case 1:
                    variant.listModel = q10;
                    break;
                default:
                case 0:
                    variant.listModel = z10;
                    break;
                }

                variant.selectedItem = 0; variant.text = variant.listModel.get(0).name;
            }
        }
        TextCoupleSelect {
            id: variant
            z: 10
            type: "Variant"
            selectedItem: 1
            initialText: "STL 100-2"
            ListModel {
                id: z10
                ListElement { name: "STL 100-1" }
                ListElement { name: "STL 100-2" }
                ListElement { name: "STL 100-3" }
                ListElement { name: "STL 100-4" }
            }
            ListModel {
                id: q10
                ListElement { name: "SQN 100-1" }
                ListElement { name: "SQN 100-2" }
                ListElement { name: "SQN 100-3" }
                ListElement { name: "SQN 100-4" }
                ListElement { name: "SQN 100-5" }
            }
            ListModel {
                id: r10
                ListElement { name: "SQR 100-1" }
                ListElement { name: "SQR 100-2" }
                ListElement { name: "SQR 100-3" }
            }

            ListModel {
                id: devalpha
                ListElement { name: "A" }
                ListElement { name: "B" }
                ListElement { name: "C" }
            }
            ListModel {
                id: playbook
                ListElement { name: "Wifi" }
                ListElement { name: "4G" }
            }
            listModel: z10
            onExpanded: { device.close(); mode.close(); }
        }
        TextCoupleSelect {
            id: mode
            z: 9
            type: "Mode"
            initialText: "Upgrade"
            listModel: ListModel {
                ListElement { name: "Upgrade" }
                ListElement { name: "Debrick" }
            }
            onExpanded: { device.close(); variant.close(); }
        }
        TextCoupleSelect {
            id: server
            z: 8
            type: "Server"
            initialText: "CSE Production"
            listModel: ListModel {
                ListElement { name: "CSE Production" }
                ListElement { name: "CSE Beta" }
                ListElement { name: "CSE BBWorld" }
            }
            onExpanded: { device.close(); mode.close(); variant.close(); }
        }
        /*TextCoupleSelect {
            id: version
            z: 7
            type: "API"
            initialText: "2.1.0"
            listModel: ListModel {
                ListElement { name: "2.1.0" }
                ListElement { name: "2.0.0" }
                ListElement { name: "1.0.0" }
            }
            onExpanded: { device.close(); mode.close(); variant.close(); }
        }*/
    }
    Rectangle {
        anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom; bottomMargin: 80 }
        width: 400
        Text {
            anchors { horizontalCenter: parent.horizontalCenter; }
            property string message: p.error
            visible: message.length > 1
            font.bold: true
            font.pointSize: 8
            text: (message.length < 5) ? "Server did not respond as expected [" + message + "]." : (message === "Success" ? "Success. No updates were available." : message)
        }
    }
    Rectangle {
        id: updateMessage
        anchors {top: parent.top; topMargin: 30; left: parent.left; leftMargin: 200 }
        color: pal.window
        border.color: pal.shadow
        border.width: 2
        width: parent.width - 220 - Math.floor(parent.width / 100); height: parent.height - 130
        Flickable {
            id: updateFlick
            width: parent.width; height: parent.height
            contentHeight: updateText.height
            contentY: Math.floor(updateScrollBox.val*(contentHeight - height))
            TextEdit {
                id: updateText
                width: parent.width - 2
                anchors.left: parent.left; anchors.leftMargin: 3
                selectByMouse: true
                wrapMode: TextEdit.WrapAnywhere
                readOnly: true
                text: "<b>Update " + p.versionRelease + " available!</b><br>" +
                      (p.versionOS !== "" ? ("<b> OS: " + p.versionOS + "</b>") : "") +
                      (p.versionRadio !== "" ? (" + <b> Radio: " + p.versionRadio + "</b>") : "") +
                      "<br><br>" + p.description + "<br><b>Base URL<br></b>" + p.url + "<br><b>Files<br></b>" + p.applications + "<br>";
            }
        }
        clip: true
    }
    Rectangle {
        id: updateScroll
        anchors.left: updateMessage.right
        anchors.top: updateMessage.top
        width: 14 + Math.floor(parent.width / 100); height: updateMessage.height
        border.width: 2
        Rectangle {
            id: updateScrollBox
            anchors {horizontalCenter: parent.horizontalCenter }
            width: parent.width - 2; height: 16
            property double val: (y - 1) / (parent.height - height - 2)
            y: 1
            color: pal.dark
        }
        MouseArea {
            anchors.fill: parent
            onMouseYChanged: {
                if (mouseY < 1) updateScrollBox.y = 1
                else if (mouseY > updateScroll.height - updateScrollBox.height - 1) updateScrollBox.y = updateScroll.height - updateScrollBox.height - 1
                else updateScrollBox.y = mouseY
            }
        }
    }

    states: [
        State {
            name: "initing"
            when: init
            AnchorChanges { target: variables; anchors.horizontalCenter: parent.horizontalCenter; anchors.left: undefined }
            AnchorChanges { target: scanButton; anchors.horizontalCenter: parent.horizontalCenter; anchors.left: undefined }
            PropertyChanges { target: updateMessage; visible: false; opacity: 0.0; scale: 0.4 }
            PropertyChanges { target: updateScroll; visible: false; opacity: 0.0; scale: 0.4 }
            PropertyChanges { target: urlLinks; visible: false; opacity: 0.0; scale: 0.4 }
        },
        State {
            name: "showing"
            when: !init
            AnchorChanges { target: variables; anchors.horizontalCenter: undefined; anchors.left: parent.left }
            AnchorChanges { target: scanButton; anchors.horizontalCenter: undefined; anchors.left: parent.left }
            PropertyChanges { target: updateMessage; visible: true; opacity: 1.0; scale: 1.0 }
            PropertyChanges { target: updateScroll; visible: true; opacity: 1.0; scale: 1.0 }
            PropertyChanges { target: urlLinks; visible: true; opacity: 1.0; scale: 1.0 }
        }
    ]

    transitions:
        Transition {
                from: "initing, showing, error"
                AnchorAnimation { duration: 200 }
                PropertyAnimation { property: "opacity"; duration: 200 }
                PropertyAnimation { property: "scale"; duration: 200 }
            }
}
