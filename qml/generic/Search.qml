import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import QtQuick.Window 2.1
import "UI" 1.0


Item {
    property bool init: p.updateMessage === ""
    state: "initing"

    Window {
        id: downloadWin
        property int maxId: download.maxId
        onMaxIdChanged: visible = (maxId > 0)
        visible: false
        onVisibleChanged: if (visible) {
                              x = window.x + (window.width - width) / 2
                              y = window.y + (window.height - height) / 2
                          }
        width: parent.width / 3; height: Math.min(parent.height / 2, width + 20);
        color: "lightgray"
        title: qsTr("Download")

        ColumnLayout {
            anchors.fill: parent
            CircleProgress {
                Layout.fillHeight: true
                Layout.fillWidth: true
                currentValue: download.curProgress
                overallValue: download.progress
                curId: download.id + 1
                maxId: download.maxId
                text: download.curName
            }
            Button {
                id: cancelButton
                text:  qsTr("Cancel Download")
                onClicked: download.reset();
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
    Column {
        id: scanButton
        anchors { bottom: parent.bottom; bottomMargin: 15; leftMargin: 15 }
        ColumnLayout {
            anchors.horizontalCenter: parent.horizontalCenter
            Button {
                id: searchButton
                anchors.horizontalCenter: parent.horizontalCenter
                enabled: !p.scanning
                text: p.scanning ? qsTr("Searching...") : qsTr("Search")
                onClicked: { p.updateDetailRequest(delta.checked ? i.appDeltaMsg : "", country.value, carrier.value, device.selectedItem, variant.selectedItem, mode.selectedItem, server.selectedItem  /*, version.selectedItem*/) }
            }
            RadioButton {
                id: delta
                anchors.horizontalCenter: parent.horizontalCenter
                visible: !p.scanning && typeof i !== 'undefined' && i.appCount > 0
                text:  qsTr("Delta")
            }
            Button {
                text:  qsTr("Version Lookup")
                onClicked: versionLookup.visible = !versionLookup.visible
            }
        }
        Text {
            property string message: p.error
            visible: message.length > 1 && !p.multiscan && p.updateMessage == ""
            Layout.alignment: Qt.AlignHCenter
            font.bold: true
            onMessageChanged: if (message.length && message.length < 5)
                                  text = qsTr("Server did not respond as expected [") + message + "]."
                              else if (message === "Success")
                                  text = qsTr("Success. No updates were available.")
                              else
                                  text = message;
        }
    }
    RowLayout {
        id: urlLinks
        anchors { left: variables.right; right: parent.right; bottom: parent.bottom; margins: 15 }
        GroupBox {
            title: qsTr("Download For")
            visible: !blackberry
            TextCoupleSelect {
                id: downloadDevice
                type: qsTr("Device")
                selectedItem: 0

                //property int familyType: (selectedItem == 0) ? i.knownHWFamily : selectedItem
                property string hwid: typeof i == 'undefined' ? "" : i.knownHW
                property int hwfam: typeof i == 'undefined' ? "" : i.knownHWFamily
                property string familyName: (hwfam == 0 || hwfam > listModel.count) ? qsTr("Unknown") : listModel.get(hwfam).text
                subtext: hwid != "" ? hwid + " (" + familyName + ")" : ""
                onSubtextChanged: generateModel()
                Component.onCompleted: generateModel()

                function generateModel() {
                    var newText = (hwid != "Unknown" && hwid != "") ? qsTr("Connected") : qsTr("As Searched")
                    if (listModel.count < 2 || listModel.get(0).text !== newText) {
                        listModel.clear()
                        listModel.append({"text" : newText })
                        for (var i = 0; i < babyModel.count; i++)
                            listModel.append({"text" : babyModel.get(i).text })
                    }
                }

                listModel: ListModel {
                    ListElement { text:  "As Searched" }
                }
            }
        }
        ColumnLayout {
            Button {
                enabled: p.updateCheckedCount > 0
                Layout.alignment: Qt.AlignHCenter
                text:  qsTr("Grab Links")
                onClicked: p.grabLinks(downloadDevice.selectedItem)
            }
            Button {
                enabled: p.updateCheckedAvailableCount > 0 && !download.verifying
                visible: !download.running
                Layout.alignment: Qt.AlignHCenter
                text: download.verifying ? qsTr("Verifying") : qsTr("Download")
                onClicked: { download.start(); p.downloadLinks(downloadDevice.selectedItem) }
            }
            Button {
                visible: download.running
                text:  qsTr("View Download (") + download.progress + "%)"
                onClicked: downloadWin.visible = true
            }
            Button {
                visible: download.running
                text:  qsTr("Cancel Download")
                onClicked: download.reset();
            }
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
            type: qsTr("Country")
            value: "302"
            subtext: carrierinfo.country
            restrictions: Qt.ImhDigitsOnly | Qt.ImhNoPredictiveText
            maxLength: 3
            onValueChanged: if (value.length == 3) carrierinfo.mccChange(value);
            onClicked: searchButton.clicked();
            helpLink: "https://en.wikipedia.org/w/index.php?title=Mobile_country_code"
        }
        TextCouple {
            id: carrier
            type: qsTr("Carrier")
            value: "720"
            subtext: carrierinfo.carrier
            restrictions: Qt.ImhDigitsOnly | Qt.ImhNoPredictiveText
            maxLength: 3
            onValueChanged: if (value.length > 0 && value.length <= 3) carrierinfo.mncChange(("00" + value).slice(-3));
            onClicked: searchButton.clicked();
        }
        Image {
            source: carrierinfo.image <= 0 ? "" : "http://appworld.blackberry.com/ClientAPI/image/" + carrierinfo.image + "/150X/png"
            sourceSize {height: carrier.height * 2 }
        }

        GroupBox {
            title: qsTr("Search For")
            ColumnLayout {
                TextCoupleSelect {
                    id: device
                    selectedItem: 4
                    type: qsTr("Device")

                    // List everything we know except abandoned models
                    ListModel {
                        id: advancedModel
                        ListElement { text:  "Z30 + Classic"}
                        ListElement { text:  "Z10 (OMAP)" }
                        ListElement { text:  "Z10 (QCOM) + P9982" }
                        ListElement { text:  "Z3 + Cafe" }
                        ListElement { text:  "Passport" }
                        ListElement { text:  "Q5 + Q10 + P9983" }
                        ListElement { text:  "Developer" }
                        ListElement { text:  "Ontario"}
                    }
                    // Only list released models
                    ListModel {
                        id: babyModel
                        ListElement { text:  "Z30" }
                        ListElement { text:  "Z10 (STL 100-1)" }
                        ListElement { text:  "Z10 (STL 100-2/3/4) + P9982" }
                        ListElement { text:  "Z3"}
                        ListElement { text:  "Passport"}
                        ListElement { text:  "Q5 + Q10 + P9983"}
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
                                variantModel.append({ 'text': qsTr('Any')});
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
                    type: qsTr("Variant")
                    selectedItem: 0
                    // This is going to be hell to maintain. Maybe an identifier in dev[] for carrier-specific and its associated code?
                    /*onSelectedItemChanged: if (device.text === "Z10 QCOM" && selectedItem == 3) { country.value = "311"; carrier.value = "480" }
                                           else if (device.text === "Q10" && selectedItem == 2) { country.value = "311"; carrier.value = "480" }
                                           else if (device.text === "Q10" && selectedItem == 4) { country.value = "310"; carrier.value = "120" }
                                           else if (device.text === "Z30" && selectedItem == 3) { country.value = "311"; carrier.value = "480" }
                                           else if (device.text === "Z30" && selectedItem == 4) { country.value = "310"; carrier.value = "120" }*/

                    listModel: ListModel { id: variantModel; }
                }
            }
        }

        TextCoupleSelect {
            id: mode
            type: qsTr("Mode")
            listModel: [ qsTr("Upgrade"), qsTr("Debrick") ]
        }

        TextCoupleSelect {
            visible: settings.advanced
            id: server
            type: qsTr("Server")
            listModel: [ qsTr("Production"), qsTr("Beta"), qsTr("Beta 2"), qsTr("Alpha"), qsTr("Alpha 2") ]
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
            title:  qsTr("Options")
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
        TableViewColumn { width: parent.width - verCol.width - sizeCol.width; id: nameCol; role: "friendlyName"; title:  qsTr("Name" )}
        TableViewColumn { id: verCol;  role: "version"; title:  qsTr("Version"; resizable: false); }
        TableViewColumn { id: sizeCol;  role: "size"; title:  qsTr("Size"); resizable: false; }
        //onModelChanged: { verCol.resizeToContents(); sizeCol.resizeToContents(); }
        rowDelegate: Rectangle {
            opacity: 0.2
            color: { switch(typeof modelData != 'undefined' ? modelData.type : "") {
                case "os": return "red";
                case "radio": return "purple";
                case "application": if (modelData.friendlyName.indexOf("sys.data") === 0) return "lightblue"; else  return "steelblue";
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
    Label { visible: false; id: sizeHint; font.pointSize: 12; text:  qsTr("1700.0 MB"); }
    GroupBox {
        id: updateAppMessage
        // Qt 5.2 width bug: Add an extra 8 spaces to message to compensate
        property string selectedMsg: qsTr("Selected: ") + ((p.updateCheckedCount == p.updateAppCount) ? qsTr("All (") + p.updateAppCount + ")" : p.updateCheckedCount + "/" + p.updateAppCount) + qsTr(" Apps")
                                     + ((p.updateAvailableCount !== p.updateAppCount) ? qsTr(". Needed: ") + ((p.updateCheckedAvailableCount == p.updateAppAvailableCount) ? qsTr("All (") + p.updateAppAvailableCount + ")" : p.updateCheckedAvailableCount + "/" + p.updateAppAvailableCount + qsTr(" Apps")) : "") + "        "
        title: !blackberry ? selectedMsg : ""

        anchors {top: updateMessage.bottom; bottom: urlLinks.top; left: variables.right; right: parent.right; margins: 15; }
        Layout.fillHeight: true
        Layout.fillWidth: true
        ScrollView {
            anchors.fill: parent
            ListView {
                anchors.fill: parent
                spacing: 3
                clip: true
                model: p.updateAppList
                Menu {
                    id: options_menu
                    signal checkAll()
                    signal checkAllAvailable()
                    signal uncheckAll()
                    title:  qsTr("Options")
                    MenuItem {
                        enabled: p.updateCheckedCount !== p.updateAppCount
                        text:  qsTr("Check All")
                        onTriggered: {
                            options_menu.checkAll();
                            for (var i = 0; i < p.updateAppCount; i++)
                                p.updateAppList[i].isMarked = true;
                        }
                    }
                    MenuItem {
                        enabled: p.updateCheckedAvailableCount !== p.updateAppAvailableCount
                        text:  qsTr("Check All Needed")
                        onTriggered: {
                            options_menu.checkAllAvailable();
                            for (var i = 0; i < p.updateAppCount; i++)
                                p.updateAppList[i].isMarked = p.updateAppList[i].isAvailable;
                        }
                    }
                    MenuItem {
                        enabled: p.updateCheckedCount > 0
                        text:  qsTr("Uncheck All")
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
                            case "radio": return "purple";
                            case "application": if (friendlyName.indexOf("sys.data") === 0) return "lightblue"; else  return "steelblue";
                            default: return "transparent";
                            }
                        }
                        opacity: 0.2
                    }
                    CheckBox {
                        id: delegateBox
                        text: friendlyName + (isAvailable ? "" : qsTr(" (downloaded)"))
                        width: Math.min(implicitWidth, parent.width - versionText.width*versionText.visible - sizeText.width)
                        clip: true
                        checked: isMarked
                        onCheckedChanged: isMarked = checked;
                        Connections {
                            target: options_menu
                            onCheckAll: delegateBox.checked = true;
                            onCheckAllAvailable: delegateBox.checked = isAvailable;
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
                        text: (size / 1024 / 1024).toFixed(1) + qsTr(" MB")
                        font.pointSize: 12;
                    }
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
