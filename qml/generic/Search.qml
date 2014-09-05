import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import "mcc.js" as MCC
import "UI" 1.0


Item {
    property bool init: p.versionRelease === ""
    property bool isMobile: false
    state: "initing"

    Rectangle {
        visible: p.downloading
        anchors {bottom: parent.bottom; bottomMargin: 10; horizontalCenter: parent.horizontalCenter }
        height: 115; width: parent.width - 20; radius: 5
        color: "lightgray"
        z: 5
        opacity: 0.95
        Text {
            text: "Download"
            font.pointSize: 10
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Text {
            id: dlText
            anchors {top: parent.top; topMargin: 40; left: parent.left; leftMargin: 10 }
            property int thisId: p.currentId + 1
            text: "Downloading (" + thisId + " of " + p.maxId + "): "
            font.pointSize: 10
        }
        Rectangle {
            anchors {left: dlText.right; leftMargin: 10; verticalCenter: dlText.verticalCenter }
            color: "transparent"
            width: 240; height: 40
            border { color: "gray"; width: 1 }
            Rectangle {
                x: 1; y: 1
                width: (p.dlProgress / 100) * parent.width - 2
                height: 40 - 2
                color: "lightsteelblue"
            }
            Text {
                text: p.currentFile
                anchors {top: parent.top; topMargin: 2; horizontalCenter: parent.horizontalCenter }
                font.pointSize: 10
            }
            Text {
                anchors {bottom: parent.bottom; bottomMargin: 2; horizontalCenter: parent.horizontalCenter }
                text: "("+p.dlProgress+"%)"
                font.pointSize: 10
            }
        }
        Button {
            anchors {bottom: parent.bottom; bottomMargin: 10; horizontalCenter: parent.horizontalCenter}
            text: "Cancel"
            onClicked: p.abortDL()
        }
    }
    Column {
        id: scanButton
        anchors { bottom: parent.bottom; bottomMargin: 60; leftMargin: 60 }
        ColumnLayout {
            anchors.horizontalCenter: parent.horizontalCenter
            RowLayout {
                anchors.horizontalCenter: parent.horizontalCenter
                /*RadioButton {
                id: delta
                visible: !p.scanning && typeof i !== 'undefined' && i.appCount > 0
                text: "Delta"
            }*/
                Button {
                    id: searchButton
                    enabled: !p.scanning
                    text: p.scanning ? "Searching..." : "Search"
                    onClicked: { p.updateDetailRequest(/*delta.checked ? i.appDeltaMsg :*/ "", country.value, carrier.value, device.selectedItem, variant.selectedItem, mode.selectedItem, server.selectedItem  /*, version.selectedItem*/) }
                }

            }
            Button {
                text: "Version Lookup"
                onClicked: versionLookup.visible = !versionLookup.visible
            }
        }
        Text {
            property string message: p.error
            visible: message.length > 1
            Layout.alignment: Qt.AlignHCenter
            font.bold: true
            text: (message.length < 5) ? "Server did not respond as expected [" + message + "]." : (message === "Success" ? "Success. No updates were available." : message)
        }
    }
    ColumnLayout {
        id: urlLinks
        anchors { right: parent.right; rightMargin: 60; bottom: parent.bottom; bottomMargin: 60 }
        Button {
            Layout.alignment: Qt.AlignHCenter
            text: isMobile ? "Copy Links" : "Grab Links"
            onClicked: p.grabLinks()
        }
        Button {
            Layout.alignment: Qt.AlignHCenter
            text: "Download All"
            onClicked: {p.dlProgress = -1; p.downloadLinks() }
        }
    }
    ColumnLayout {
        id: variables
        anchors.top: parent.top
        anchors.topMargin: 30
        anchors.leftMargin: 10
        Layout.fillHeight: true
        height: (parent.height * 4) / 6
        TextCouple {
            id: country
            type: "Country"
            subtext: "Germany"
            value: "262"
            restrictions: Qt.ImhDigitsOnly | Qt.ImhNoPredictiveText
            maxLength: 3
            onValueChanged: {
                if (value.length == 3) subtext = MCC.to_country(value);
                if (carrier != null) carrier.updateVal();
            }
            onClicked: searchButton.clicked();
            helpLink: "https://en.wikipedia.org/w/index.php?title=Mobile_country_code"
        }
        TextCouple {
            id: carrier
            type: "Carrier"
            value: "001"
            restrictions: Qt.ImhDigitsOnly | Qt.ImhNoPredictiveText
            maxLength: 3
            function updateVal() {
                if (value.length <= 3) subtext = MCC.to_carrier(country.value, ("00" + value).slice(-3)); else subtext = "";
            }

            onValueChanged: updateVal();
            onClicked: searchButton.clicked();
        }
        TextCoupleSelect {
            id: device
            z: 11
            selectedItem: 6
            type: "Device"
            listModel: p.advanced ? advancedModel : babyModel
            ListModel {
                id: advancedModel
                ListElement { text: "Z30" }
                ListElement { text: "Z10 OMAP" }
                ListElement { text: "Z10 QCOM" }
                ListElement { text: "P9982" }
                ListElement { text: "Z3" }
                ListElement { text: "Q30" }
                ListElement { text: "Q10" }
                ListElement { text: "Q5" }
                ListElement { text: "Z5" }
                ListElement { text: "Q3" }
                ListElement { text: "Dev Alpha" }
                ListElement { text: "Playbook" }
                ListElement { text: "Ontario" }
                ListElement { text: "Classic" }
                ListElement { text: "Khan" }
            }
            ListModel {
                id: babyModel
                ListElement { text: "Z30" }
                ListElement { text: "Z10 OMAP" }
                ListElement { text: "Z10 QCOM" }
                ListElement { text: "P9982" }
                ListElement { text: "Z3" }
                ListElement { text: "Q30" }
                ListElement { text: "Q10" }
                ListElement { text: "Q5" }
            }
            onSelectedItemChanged: if (variantModel != null) {
                                       variantModel.clear()
                                       if (p.variantCount(selectedItem) > 1)
                                           variantModel.append({ 'text': 'Any'});
                                       for (var i = 0; i < p.variantCount(selectedItem); i++)
                                           variantModel.append({ 'text': p.nameFromVariant(selectedItem, i)})

                                       variant.selectedItem = 0;
                                   }
        }

        TextCoupleSelect {
            visible: p.advanced
            id: variant
            z: 10
            type: "Variant"
            selectedItem: 0
            subtext: i.knownHW != "" ? "Connected: " + i.knownHW : ""
            onSelectedItemChanged: if (device.text === "Z10 QCOM" && selectedItem == 3) { country.value = "311"; carrier.value = "480" }
                                   else if (device.text === "Q10" && selectedItem == 2) { country.value = "311"; carrier.value = "480" }
                                   else if (device.text === "Q10" && selectedItem == 4) { country.value = "310"; carrier.value = "120" }
                                   else if (device.text === "Z30" && selectedItem == 3) { country.value = "311"; carrier.value = "480" }
                                   else if (device.text === "Z30" && selectedItem == 4) { country.value = "310"; carrier.value = "120" }

            listModel: ListModel { id: variantModel; ListElement { text:  "STJ 100-1" } }
        }
        TextCoupleSelect {
            visible: p.advanced
            id: mode
            z: 9
            type: "Mode"
            listModel: [ "Upgrade", "Debrick" ]
        }

        TextCoupleSelect {
            visible: p.advanced
            id: server
            z: 8
            type: "Server"
            listModel: [ "Production", "Beta" ]
        }

        /*TextCoupleSelect {
            id: version
            z: 7
            type: "API"
            listModel: [ "2.1.0", "2.0.0", "1.0.0" ]
        }*/
    }
    VersionLookup {
        id: versionLookup
    }

    TextArea {
        id: updateMessage
        anchors {top: parent.top; bottom: urlLinks.top; left: variables.right; right: parent.right; margins: 30; }
        //width: parent.width - 200; height: parent.height - 130
        text: "<b>Update " + p.versionRelease + " available for " + p.variant + "!</b><br>" +
              (p.versionOS !== "" ? ("<b> OS: " + p.versionOS + "</b>") : "") +
              (p.versionRadio !== "" ? (" + <b> Radio: " + p.versionRadio + "</b>") : "") +
              "<br><br>" + p.description + "<br><b>Base URL<br></b>" + p.updateUrl + "<br><b>Files<br></b>" + p.applications + "<br>";
        readOnly: true
        textFormat: TextEdit.RichText
        selectByKeyboard: true
    }

    states: [
        State {
            name: "initing"
            when: init
            AnchorChanges { target: variables; anchors.horizontalCenter: parent.horizontalCenter; anchors.left: undefined }
            AnchorChanges { target: scanButton; anchors.horizontalCenter: parent.horizontalCenter; anchors.left: undefined }
            PropertyChanges { target: updateMessage; visible: false; opacity: 0.0; scale: 0.4 }
            PropertyChanges { target: urlLinks; visible: false; opacity: 0.0; scale: 0.4 }
        },
        State {
            name: "showing"
            when: !init
            AnchorChanges { target: variables; anchors.horizontalCenter: undefined; anchors.left: parent.left }
            AnchorChanges { target: scanButton; anchors.horizontalCenter: undefined; anchors.left: parent.left }
            PropertyChanges { target: updateMessage; visible: true; opacity: 1.0; scale: 1.0 }
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
