import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import "mcc.js" as MCC
import "UI" 1.0


Item {
    property bool init: p.updateMessage === ""
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
        anchors { bottom: parent.bottom; bottomMargin: 15; leftMargin: 15 }
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
            visible: message.length > 1 && !p.multiscan && p.updateMessage == ""
            Layout.alignment: Qt.AlignHCenter
            font.bold: true
            onMessageChanged: if (message.length && message.length < 5)
                                  text = "Server did not respond as expected [" + message + "]."
                              else if (message === "Success")
                                  text = "Success. No updates were available."
                              else
                                  text = message;
        }
    }
    ColumnLayout {
        id: urlLinks
        anchors { right: parent.right; rightMargin: 15; bottom: parent.bottom; bottomMargin: 15 }
        Button {
            enabled: p.updateCheckedCount > 0
            Layout.alignment: Qt.AlignHCenter
            text: isMobile ? "Copy Links" : "Grab Links"
            onClicked: p.grabLinks(downloadDevice.selectedItem)
        }
        Button {
            enabled: p.updateCheckedCount > 0
            Layout.alignment: Qt.AlignHCenter
            text: (p.updateCheckedCount == p.updateAppCount) ? "Download All" : "Download Selected (" + p.updateCheckedCount + ")"
            onClicked: {p.dlProgress = -1; p.downloadLinks(downloadDevice.selectedItem) }
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
            subtext: "Australia"
            value: "505"
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
            value: "002"
            restrictions: Qt.ImhDigitsOnly | Qt.ImhNoPredictiveText
            maxLength: 3
            function updateVal() {
                if (value.length <= 3) subtext = MCC.to_carrier(country.value, ("00" + value).slice(-3)); else subtext = "";
            }

            onValueChanged: updateVal();
            onClicked: searchButton.clicked();
        }
        GroupBox {
            title: "Search Device"
            ColumnLayout {
                TextCoupleSelect {
                    id: device
                    selectedItem: 4
                    type: "Device"

                    ListModel {
                        id: advancedModel
                        ListElement { text: "Z30" }
                        ListElement { text: "Z10 (OMAP)" }
                        ListElement { text: "Z10 (QCOM) + P9982" }
                        ListElement { text: "Z3" }
                        ListElement { text: "Passport" }
                        ListElement { text: "Q5 + Q10" }
                        ListElement { text: "Z5" }
                        ListElement { text: "Q3" }
                        ListElement { text: "Developer" }
                        ListElement { text: "Ontario" }
                        ListElement { text: "Classic" }
                        ListElement { text: "Khan" }
                    }
                    ListModel {
                        id: babyModel
                        ListElement { text: "Z30" }
                        ListElement { text: "Z10 (OMAP)" }
                        ListElement { text: "Z10 (QCOM) + P9982" }
                        ListElement { text: "Z3" }
                        ListElement { text: "Passport" }
                        ListElement { text: "Q5 + Q10" }
                    }
                    function changeModel() {
                        var selected = selectedItem
                        listModel = settings.advanced ? advancedModel : babyModel
                        selectedItem = Math.min(selected, listModel.count - 1);
                    }
                    function updateVariant() {
                        if (variantModel != null) {
                            variantModel.clear()
                            if (p.variantCount(selectedItem) > 1)
                                variantModel.append({ 'text': 'Any'});
                            for (var i = 0; i < p.variantCount(selectedItem); i++)
                                variantModel.append({ 'text': p.nameFromVariant(selectedItem, i)})

                            variant.selectedItem = 0;
                        }
                    }
                    property bool advanced: settings.advanced
                    onAdvancedChanged: changeModel()
                    onSelectedItemChanged: updateVariant();
                    Component.onCompleted: { changeModel(); updateVariant(); }
                }

                TextCoupleSelect {
                    visible: settings.advanced
                    id: variant
                    type: "Variant"
                    selectedItem: 0
                    onSelectedItemChanged: if (device.text === "Z10 QCOM" && selectedItem == 3) { country.value = "311"; carrier.value = "480" }
                                           else if (device.text === "Q10" && selectedItem == 2) { country.value = "311"; carrier.value = "480" }
                                           else if (device.text === "Q10" && selectedItem == 4) { country.value = "310"; carrier.value = "120" }
                                           else if (device.text === "Z30" && selectedItem == 3) { country.value = "311"; carrier.value = "480" }
                                           else if (device.text === "Z30" && selectedItem == 4) { country.value = "310"; carrier.value = "120" }

                    listModel: ListModel { id: variantModel; }
                }
            }
        }
        GroupBox {
            title: "Download Device"
            TextCoupleSelect {
                id: downloadDevice
                type: "Device"
                selectedItem: 0

                //property int familyType: (selectedItem == 0) ? i.knownHWFamily : selectedItem
                property string familyName: i.knownHWFamily == 0 ? "Unknown" : listModel.get(i.knownHWFamily).text
                subtext: i.knownHW != "" ? i.knownHW + " (" + familyName + ")" : ""
                onSubtextChanged: {
                    var newText = (i.knownHW != "Unknown" && i.knownHW != "") ? "Connected" : "As Above"
                    if (listModel.get(0).text !== newText) {
                        listModel.remove(0, 1)
                        listModel.insert(0, {"text" : newText })
                    }
                }
                listModel: ListModel {
                    ListElement { text: "As above" }
                    ListElement { text: "Z30" }
                    ListElement { text: "Z10 (OMAP)" }
                    ListElement { text: "Z10 (QCOM) + P9982" }
                    ListElement { text: "Z3" }
                    ListElement { text: "Passport" }
                    ListElement { text: "Q5 + Q10" }
                }
            }
        }

        TextCoupleSelect {
            id: mode
            type: "Mode"
            listModel: [ "Upgrade", "Debrick" ]
        }

        TextCoupleSelect {
            visible: settings.advanced
            id: server
            type: "Server"
            listModel: [ "Production", "Beta" ]
        }

        /*TextCoupleSelect {
            id: version
            type: "API"
            listModel: [ "2.1.0", "2.0.0", "1.0.0" ]
        }*/
    }
    VersionLookup {
        id: versionLookup
    }

    TextArea {
        id: updateMessage
        anchors {top: parent.top; left: variables.right; right: parent.right; margins: 15; }
        Layout.fillWidth: true
        height: parent.height / 10
        text: p.updateMessage
        readOnly: true
        textFormat: TextEdit.RichText
        selectByKeyboard: true
    }

    // Changes required to make this workable
    // - Need to prevent it being horizontally scrollable or at least make it fit by default
    // - Requires Qt 5.3 for resizeToContents()!!
    // - Context menu doesn't work? We need (Un)Check All
    /*TableView {
        id: updateAppMessage
        anchors {top: updateMessage.bottom; bottom: urlLinks.top; left: variables.right; right: parent.right; margins: 15; }
        Layout.fillHeight: true
        Layout.fillWidth: true
        alternatingRowColors: false
        backgroundVisible: false
        model: p.updateAppList
        Menu {
            id: options_menu
            signal checkAll()
            signal uncheckAll()
            title: "Options"
            MenuItem {
                enabled: p.updateCheckedCount != p.updateAppCount
                text: "Check All"
                onTriggered: {
                    options_menu.checkAll();
                    for (var i = 0; i < p.updateAppCount; i++)
                        p.updateAppList[i].isMarked = true;
                }
            }
            MenuItem {
                enabled: p.updateCheckedCount > 0
                text: "Uncheck All"
                onTriggered: {
                    options_menu.uncheckAll()
                    for (var i = 0; i < p.updateAppCount; i++)
                        p.updateAppList[i].isMarked = false;
                }
            }
        }
        TableViewColumn { width: parent.width - verCol.width - sizeCol.width; id: nameCol; role: "friendlyName"; title: "Name" }
        TableViewColumn { id: verCol;  role: "version"; title: "Version"; resizable: false; }
        TableViewColumn { id: sizeCol;  role: "size"; title: "Size"; resizable: false; }
        //onModelChanged: { verCol.resizeToContents(); sizeCol.resizeToContents(); }
        rowDelegate: Rectangle {
            opacity: 0.2
            color: { switch(typeof modelData != 'undefined' ? modelData.type : "") {
                case "os": return "red";
                case "radio": return "maroon";
                case "application": if (modelData.friendlyName.indexOf("sys.data") === 0) return "purple"; else  return "steelblue";
                default: return "transparent";
                }
            }
        }

        itemDelegate: Text {
            property variant value: styleData.value
            text: (styleData.role === "size") ? (value / 1024 / 1024).toFixed(1) + " MB" : value
            horizontalAlignment: (styleData.role === "size") ? Qt.AlignRight : Qt.AlignLeft
            clip: true
        }
    }*/

    // Cheat to get system widths of text here. Should use a TableView (above) later to replace it.
    // A Label with 6 characters and ' MB', like the maximum filesize of an app
    Label { visible: false; id: sizeHint; font.pointSize: 12; text: "1700.0 MB"; }
    ScrollView {
        id: updateAppMessage
        anchors {top: updateMessage.bottom; bottom: urlLinks.top; left: variables.right; right: parent.right; margins: 15; }
        Layout.fillHeight: true
        Layout.fillWidth: true
        ListView {
            anchors.fill: parent
            spacing: 3
            clip: true
            model: p.updateAppList
            Menu {
                id: options_menu
                signal checkAll()
                signal uncheckAll()
                title: "Options"
                MenuItem {
                    enabled: p.updateCheckedCount != p.updateAppCount
                    text: "Check All"
                    onTriggered: {
                        options_menu.checkAll();
                        for (var i = 0; i < p.updateAppCount; i++)
                            p.updateAppList[i].isMarked = true;
                    }
                }
                MenuItem {
                    enabled: p.updateCheckedCount > 0
                    text: "Uncheck All"
                    onTriggered: {
                        options_menu.uncheckAll()
                        for (var i = 0; i < p.updateAppCount; i++)
                            p.updateAppList[i].isMarked = false;
                    }
                }
            }

            MouseArea {
                acceptedButtons: Qt.RightButton
                onClicked: options_menu.popup()
                anchors.fill: parent
            }
            delegate: Item {
                visible: type !== "";
                width: parent.width - 3
                height: type === "" ? 0 : 26
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
                    id: delegateBox
                    text: friendlyName
                    width: Math.min(implicitWidth, parent.width - versionText.width*versionText.visible - sizeText.width)
                    clip: true
                    checked: isMarked
                    onCheckedChanged: isMarked = checked;
                    Connections {
                        target: options_menu
                        onCheckAll: delegateBox.checked = true;
                        onUncheckAll: delegateBox.checked = false;
                    }
                }

                Label {
                    id: versionText
                    anchors.right: sizeText.left;
                    visible: (parent.width - sizeText.paintedWidth) > delegateBox.implicitWidth
                    text: version
                }
                Label {
                    id: sizeText
                    anchors.right: parent.right
                    width: sizeHint.width
                    horizontalAlignment: Text.AlignRight
                    text: (size / 1024 / 1024).toFixed(1) + " MB"
                    font.pointSize: 12;
                }
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
            PropertyChanges { target: updateAppMessage; visible: false; opacity: 0.0; scale: 0.4 }
            PropertyChanges { target: urlLinks; visible: false; opacity: 0.0; scale: 0.4 }
        },
        State {
            name: "showing"
            when: !init
            AnchorChanges { target: variables; anchors.horizontalCenter: undefined; anchors.left: parent.left }
            AnchorChanges { target: scanButton; anchors.horizontalCenter: undefined; anchors.left: parent.left }
            PropertyChanges { target: updateMessage; visible: true; opacity: 1.0; scale: 1.0 }
            PropertyChanges { target: updateAppMessage; visible: true; opacity: 1.0; scale: 1.0 }
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
