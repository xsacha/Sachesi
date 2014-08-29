import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Window 2.2
import "mcc.js" as MCC
import "UI" 1.0

TabView {
    id: main
    property bool init: p.versionRelease === ""
    property bool isMobile: false
    state: "initing"

    Window {
        id: versionLookup
        x: window.x + (window.width - width) / 2
        y: window.y + (window.height - height) / 2
        title: window.title + " - Version Lookup"
        visible: false
        height: 12 + config.defaultFontSize * 8;
        width: 100 + config.defaultFontSize * 18;
        Column {
            anchors { left: parent.left; leftMargin: 20; top: parent.top; topMargin: 20 }
            spacing: config.defaultFontSize
            Row {
                spacing: config.defaultFontSize - 4
                Row {
                    SpinBox {
                        id: major
                        prefix: "10."
                        value: 3
                        maximumValue: 255
                        onEditingFinished: relookup.clicked()
                    }
                    SpinBox {
                        id: minor
                        value: 0
                        maximumValue: 255
                        onEditingFinished: relookup.clicked()
                    }
                    SpinBox {
                        id: build
                        value: 1052
                        maximumValue: 9999
                        stepSize: 3
                        onEditingFinished: relookup.clicked()
                    }
                }
                Button {
                    id: relookup
                    text: "Lookup"
                    enabled: !p.scanning
                    onClicked: p.reverseLookup(country.value, carrier.value, device.selectedItem, variant.selectedItem, 0/*server.selectedItem*/, "10." + major.value + "." + minor.value + "." + build.value);
                }
                Button {
                    property bool looking: false
                    text: looking ? "Stop Scan" : "Autoscan"
                    enabled: !p.scanning || looking
                    onClicked: { looking = !looking; if (looking) { build.value += 3; relookup.clicked(); } }
                    Timer {
                        id: autoLookup
                        interval: 10;
                        running: parent.looking && !p.scanning
                        onTriggered: {
                            if (p.scanning > 0)
                                return;
                            if (downloadPotential.visible) {
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
            Row {
                Text {
                    text: "Software Release: " + p.softwareRelease
                    font.pointSize: 12
                }
            }
        }
        Column {
            anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom; bottomMargin: 10 }
            spacing: config.defaultFontSize - 4
            Row {
                spacing: config.defaultFontSize
                anchors.horizontalCenter: parent.horizontalCenter
                Button {
                    id: downloadPotential
                    visible: p.softwareRelease.charAt(0) == "1" || p.softwareRelease.charAt(0) == "2"
                    property string osVersion: ""
                    onVisibleChanged: if (visible) osVersion = "10." + major.value + "." + minor.value + "." + build.value
                    enabled: true // Exists?
                    text: "Download"
                    onClicked: p.downloadPotentialLink(p.softwareRelease, osVersion)
                }
                Button {
                    visible: p.softwareRelease.charAt(0) == "1" || p.softwareRelease.charAt(0) == "2"
                    property string osVersion: ""
                    onVisibleChanged: if (visible) osVersion = "10." + major.value + "." + minor.value + "." + build.value
                    enabled: true // Exists?
                    text: isMobile ? "Copy Links" : "Grab Links"
                    onClicked: p.grabPotentialLinks(p.softwareRelease, osVersion)
                }
            }
            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Hide"
                onClicked: versionLookup.visible = false;
            }
        }
    }
    Rectangle {
        visible: p.downloading
        anchors {bottom: parent.bottom; bottomMargin: 10; horizontalCenter: parent.horizontalCenter }
        height: 100 + config.defaultSubtextSize; width: parent.width - 20; radius: 5
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
        anchors { bottom: parent.bottom; bottomMargin: 60 }
        anchors.leftMargin: 60
        spacing: 10
        Row {
            spacing: 10
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
                onClicked: { p.updateDetailRequest(/*delta.checked ? i.appDeltaMsg :*/ "", country.value, carrier.value, device.selectedItem, variant.selectedItem, mode.selectedItem, 0/*server.selectedItem*/  /*, version.selectedItem*/) }
            }

        }
        Button {
            text: "Version Lookup"
            onClicked: versionLookup.visible = !versionLookup.visible
        }
    }
    Column {
        spacing: 10
        id: urlLinks
        anchors { right: parent.right; rightMargin: 60; bottom: parent.bottom; bottomMargin: 60 }
        Button {
            text: isMobile ? "Copy Links" : "Grab Links"
            onClicked: p.grabLinks()
        }
        Button {
            text: "Download All"
            onClicked: {p.dlProgress = -1; p.downloadLinks() }
        }
    }
    Rectangle {
        anchors { horizontalCenter: parent.horizontalCenter; top: scanButton.bottom; topMargin: 1.2 * config.defaultSubtextSize }
        width: 400
        Text {
            anchors { horizontalCenter: parent.horizontalCenter; }
            property string message: p.error
            visible: message.length > 1
            font.bold: true
            font.pointSize: config.defaultSubtextSize
            text: (message.length < 5) ? "Server did not respond as expected [" + message + "]." : (message === "Success" ? "Success. No updates were available." : message)
        }
    }
    Column {
        id: variables
        spacing: 4 + (parent.height - config.defaultFontSize * 10 - 90) / 7
        anchors.top: parent.top
        anchors.topMargin: 30
        TextCouple {
            id: country
            type: "Country"
            subtext: "Indonesia"
            value: "510"
            restrictions: Qt.ImhDigitsOnly | Qt.ImhNoPredictiveText
            maxLength: 3
            onValueChanged: {
                if (value.length == 3) subtext = MCC.to_country(value);
                if (carrier != null) carrier.updateVal();
            }
            before: carrier.thisid
            after: carrier.thisid
            onClicked: searchButton.clicked();

            Rectangle {
                color: config.shadowColor
                anchors.left: country.typeOffset
                anchors.leftMargin: 5
                width: wikiLink.width; height: wikiLink.height
                radius: 4
                Text {
                    id: wikiLink
                    anchors.centerIn: parent
                    text: "?"
                    font.pointSize: 12
                    MouseArea {
                        anchors.fill: parent
                        onClicked: Qt.openUrlExternally("https://en.wikipedia.org/w/index.php?title=Mobile_country_code");
                    }
                }
            }
        }
        TextCouple {
            id: carrier
            type: "Carrier"
            value: "01"
            restrictions: Qt.ImhDigitsOnly | Qt.ImhNoPredictiveText
            maxLength: 3
            function updateVal() {
                if (value.length <= 3) subtext = MCC.to_carrier(country.value, ("00" + value).slice(-3)); else subtext = "";
            }

            onValueChanged: updateVal();
            before: country.thisid
            after: country.thisid
            onClicked: searchButton.clicked();
        }
        TextCoupleSelect {
            id: device
            z: 11
            selectedItem: 4
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
        // How to deal with OMAP STL 100-1? Currently, assume the same carrier does not carry both types.
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
        // Disabled until new Beta server code is in
        /*TextCoupleSelect {
            visible: p.advanced
            id: server
            z: 8
            type: "Server"
            listModel: [ "Production", "Beta" ]
        }*/

        /*TextCoupleSelect {
            id: version
            z: 7
            type: "API"
            listModel: [ "2.1.0", "2.0.0", "1.0.0" ]
        }*/
    }

    TextArea {
        id: updateMessage
        anchors {top: parent.top; topMargin: 30; right: parent.right; rightMargin: 34 }
        width: parent.width - 200 - parent.width / 8; height: parent.height - 130 - parent.height / 10
        text: "<b>Update " + p.versionRelease + " available for " + p.variant + "!</b><br>" +
              (p.versionOS !== "" ? ("<b> OS: " + p.versionOS + "</b>") : "") +
              (p.versionRadio !== "" ? (" + <b> Radio: " + p.versionRadio + "</b>") : "") +
              "<br><br>" + p.description + "<br><b>Base URL<br></b>" + p.url + "<br><b>Files<br></b>" + p.applications + "<br>";
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
