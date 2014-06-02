import QtQuick 1.1
import "mcc.js" as MCC
import "UI" 1.0

PageTab {
    id: main
    property bool init: p.versionRelease === ""
    property bool isMobile: false
    state: "initing"

    Rectangle {
        id: versionLookup
        opacity: 0.9
        color: "gray"

        visible: false
        x: 20; y: 20
        height: 20 + config.defaultFontSize * 10; width: 50 + config.defaultFontSize * 18; radius: 4
        z: 5
        MouseArea {
            anchors.fill: parent
            drag.target: versionLookup
            drag.axis: Drag.XandYAxis
            drag.minimumX: 20
            drag.maximumX: main.width - versionLookup.width - 20
            drag.minimumY: 20
            drag.maximumY: main.height - versionLookup.height - 20
            Column {
                anchors {left: parent.left; leftMargin: 20; top: parent.top; topMargin: 20 }
                spacing: config.defaultFontSize
                Row {
                    TextCouple {
                        id: major
                        type: "Major"
                        value: "2"
                        large: false
                        after: minor.thisid
                        before: build.thisid
                        onClicked: relookup.clicked()
                    }
                    TextCouple {
                        id: minor
                        type: "Minor"
                        value: "1"
                        large: false
                        after: build.thisid
                        before: major.thisid
                        onClicked: relookup.clicked()
                    }
                    TextCouple {
                        id: build
                        type: "Build"
                        value: "3175"
                        subtext: "Multiple of 3"
                        large: false
                        after: major.thisid
                        before: minor.thisid
                        onClicked: relookup.clicked()
                        onUpArrow: increase();
                        onDownArrow: decrease();
                        function increase() {
                            if (build.value < 9998) {
                                build.value -= -3;
                                relookup.clicked();
                            }
                        }
                        function decrease() {
                            if (build.value > 3) {
                                build.value -= 3;
                                relookup.clicked();
                            }
                        }

                        Column {
                            spacing: 10
                            anchors {left: build.right; leftMargin: -5; top: build.top; topMargin: -15 }
                            Image {
                                source: "arrow.png"
                                width: 25; height: 25
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: build.increase();
                                }
                            }
                            Image {
                                source: "arrow.png"
                                width: 25; height: 25
                                rotation: 180
                                MouseArea {
                                    id: downMouse
                                    anchors.fill: parent
                                    onClicked: build.decrease();
                                }
                            }
                        }
                    }
                }
                Row {
                    spacing: config.defaultFontSize
                    Text {
                        text: "OS: 10." + major.value + "." + minor.value + "." + build.value
                        font.pixelSize: config.defaultFontSize
                    }
                    RoundButton {
                        id: relookup
                        text: "Lookup"
                        enabled: major.value.length > 0 && minor.value.length > 0 && build.value.length > 0 && !p.scanning
                        onClicked: p.reverseLookup(country.value, carrier.value, device.selectedItem, variant.selectedItem, server.selectedItem, "10." + major.value + "." + minor.value + "." + build.value);
                    }
                    RoundButton {
                        property bool looking: false
                        text: looking ? "Stop Scan" : "Autoscan"
                        enabled: major.value.length > 0 && minor.value.length > 0 && build.value.length > 0 && (!p.scanning || looking)
                        onClicked: { looking = !looking; if (looking) { build.increase(); relookup.clicked(); } }
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
                                    build.increase();
                                    relookup.clicked();
                                }
                            }
                        }
                    }
                }
                Row {
                    Text {
                        text: "Software Release: " + p.softwareRelease
                        font.pixelSize: config.defaultFontSize
                    }
                }
            }
            Column {
                anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom; bottomMargin: 10 }
                spacing: config.defaultFontSize
                Row {
                    spacing: config.defaultFontSize
                    anchors.horizontalCenter: parent.horizontalCenter
                    RoundButton {
                        id: downloadPotential
                        visible: p.softwareRelease.charAt(0) == "1" || p.softwareRelease.charAt(0) == "2"
                        property string osVersion: ""
                        onVisibleChanged: if (visible) osVersion = "10." + major.value + "." + minor.value + "." + build.value
                        enabled: true // Exists?
                        text: "Download"
                        onClicked: p.downloadPotentialLink(p.softwareRelease, osVersion)
                    }
                    RoundButton {
                        visible: p.softwareRelease.charAt(0) == "1" || p.softwareRelease.charAt(0) == "2"
                        property string osVersion: ""
                        onVisibleChanged: if (visible) osVersion = "10." + major.value + "." + minor.value + "." + build.value
                        enabled: true // Exists?
                        text: isMobile ? "Copy Links" : "Grab Links"
                        onClicked: p.grabPotentialLinks(p.softwareRelease, osVersion)
                    }
                }
                RoundButton {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Close"
                    onClicked: versionLookup.visible = false;
                }
            }
        }
    }

    Rectangle {
        visible: p.downloading
        anchors {bottom: parent.bottom; bottomMargin: 10; horizontalCenter: parent.horizontalCenter }
        height: 100 + config.defaultSubtextSize; width: parent.width - 20; radius: 8
        z: 5
        color: "gray"
        opacity: 0.95
        Text {
            text: "Download"
            font.pixelSize: config.defaultSubtextSize
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Text {
            id: dlText
            anchors {top: parent.top; topMargin: 40; left: parent.left; leftMargin: 10 }
            property int thisId: p.currentId + 1
            text: "Downloading (" + thisId + " of " + p.maxId + "): "
            font.pixelSize: config.defaultSubtextSize
        }
        Rectangle {
            anchors {left: dlText.right; leftMargin: 10; verticalCenter: dlText.verticalCenter }
            color: "transparent"
            width: 240; height: 40
            border.width: 1;
            Rectangle {
                x: 1; y: 1
                width: (p.dlProgress / 100) * parent.width - 2
                height: 30 - 2
                color: config.darkColor
            }
            Text {
                text: p.currentFile
                anchors {top: parent.top; topMargin: 2; horizontalCenter: parent.horizontalCenter }
                font.pixelSize: config.defaultSubtextSize
            }
            Text {
                anchors {bottom: parent.bottom; bottomMargin: 2; horizontalCenter: parent.horizontalCenter }
                text: "("+p.dlProgress+"%)"
                font.pixelSize: config.defaultSubtextSize
            }
        }
        RoundButton {
            anchors {bottom: parent.bottom; bottomMargin: 10; horizontalCenter: parent.horizontalCenter}
            text: "Cancel"
            onClicked: p.abortDL()
        }
        MouseArea {
            anchors.fill: parent
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
            SelectionText {
                id: delta
                visible: !p.scanning && typeof i !== 'undefined' && i.appCount > 0
                text: "Delta"
            }
            RoundButton {
                id: searchButton
                enabled: !p.scanning
                text: p.scanning ? "Searching..." : "Search"
                onClicked: { p.updateDetailRequest(delta.checked ? i.appDeltaMsg : "", country.value, carrier.value, device.selectedItem, variant.selectedItem, mode.selectedItem, server.selectedItem/*, version.selectedItem*/) }
            }

        }
        RoundButton {
            text: "Version Lookup"
            onClicked: versionLookup.visible = !versionLookup.visible
        }
    }
    Column {
        spacing: 10
        id: urlLinks
        anchors { right: parent.right; rightMargin: 60; bottom: parent.bottom; bottomMargin: 60 }
        RoundButton {
            text: isMobile ? "Copy Links" : "Grab Links"
            onClicked: p.grabLinks()
        }
        RoundButton {
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
                    font.pixelSize: config.defaultFontSize
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
            initialText: "Z3"
            listModel: p.advanced ? advancedModel : babyModel
            ListModel {
                id: advancedModel
                ListElement { name: "Z30" }
                ListElement { name: "Z10 OMAP" }
                ListElement { name: "Z10 QCOM" }
                ListElement { name: "P9982" }
                ListElement { name: "Z3" }
                ListElement { name: "Q30" }
                ListElement { name: "Q10" }
                ListElement { name: "Q5" }
                ListElement { name: "Z5" }
                ListElement { name: "Q3" }
                ListElement { name: "Dev Alpha" }
                ListElement { name: "Playbook" }
                ListElement { name: "Ontario" }
                ListElement { name: "Classic" }
                ListElement { name: "Khan" }
            }
            ListModel {
                id: babyModel
                ListElement { name: "Z30" }
                ListElement { name: "Z10 OMAP" }
                ListElement { name: "Z10 QCOM" }
                ListElement { name: "P9982" }
                ListElement { name: "Z3" }
                ListElement { name: "Q30" }
                ListElement { name: "Q10" }
                ListElement { name: "Q5" }
            }
            onExpanded: { mode.close(); variant.close(); }
            onSelectedItemChanged: {
                variant.listModel.clear();
                if (p.variantCount(selectedItem) > 1)
                    variant.listModel.append({'name': 'Any'});
                for (var i = 0; i < p.variantCount(selectedItem); i++)
                    variant.listModel.append({'name': p.nameFromVariant(selectedItem, i)})

                variant.selectedItem = 0; variant.text = variant.listModel.get(variant.selectedItem).name;
            }
        }
        // How to deal with OMAP STL 100-1? Currently, assume the same carrier does not carry both types.
        TextCoupleSelect {
            visible: p.advanced
            id: variant
            z: 10
            type: "Variant"
            selectedItem: 0
            initialText: "STJ 100-1"
            subtext: i.knownHW != "" ? "Connected: " + i.knownHW : ""
            onSelectedItemChanged: if (device.text === "Z10" && selectedItem == 3) { country.value = "311"; carrier.value = "480" }
                                   else if (device.text === "Q10" && selectedItem == 1) { country.value = "311"; carrier.value = "480" }
                                   else if (device.text === "Q10" && selectedItem == 3) { country.value = "310"; carrier.value = "120" }
                                   else if (device.text === "Z30" && selectedItem == 2) { country.value = "311"; carrier.value = "480" }
                                   else if (device.text === "Z30" && selectedItem == 3) { country.value = "310"; carrier.value = "120" }

            listModel: ListModel { ListElement { name: "STJ 100-1" } }
            onExpanded: { device.close(); mode.close(); server.close(); }
        }
        TextCoupleSelect {
            visible: p.advanced
            id: mode
            z: 9
            type: "Mode"
            initialText: "Upgrade"
            listModel: ListModel {
                ListElement { name: "Upgrade" }
                ListElement { name: "Debrick" }
            }
            onExpanded: { device.close(); variant.close(); server.close(); }
        }
        TextCoupleSelect {
            visible: p.advanced
            id: server
            z: 8
            type: "Server"
            initialText: "Production"
            listModel: ListModel {
                ListElement { name: "Production" }
                ListElement { name: "Beta" }
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
        id: updateMessage
        anchors {top: parent.top; topMargin: 30; right: parent.right; rightMargin: 44 + Math.floor(parent.width / 100) }
        color: config.windowColor
        border.color: config.shadowColor
        border.width: 2
        width: parent.width - 100 - parent.width / 5 + Math.floor(parent.width / 100) - 5 * config.defaultFontSize; height: parent.height - 130 - parent.height / 10
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
                text: "<b>Update " + p.versionRelease + " available for " + p.variant + "!</b><br>" +
                      (p.versionOS !== "" ? ("<b> OS: " + p.versionOS + "</b>") : "") +
                      (p.versionRadio !== "" ? (" + <b> Radio: " + p.versionRadio + "</b>") : "") +
                      "<br><br>" + p.description + "<br><b>Base URL<br></b>" + p.url + "<br><b>Files<br></b>" + p.applications + "<br>";
                font.pixelSize: config.defaultSubtextSize
            }
        }
        clip: true
    }
    Rectangle {
        id: updateScroll
        anchors.left: updateMessage.right
        anchors.top: updateMessage.top
        width: config.defaultFontSize + Math.floor(parent.width / 100); height: updateMessage.height
        border.width: 2
        Rectangle {
            id: updateScrollBox
            anchors {horizontalCenter: parent.horizontalCenter }
            width: parent.width - 2; height: 32
            property double val: (y - 1) / (parent.height - height - 2)
            y: 1
            color: config.darkColor
        }
        MouseArea {
            anchors.fill: parent
            onMouseYChanged: {
                if (mouseY < 0) updateScrollBox.y = 1
                else if (mouseY > updateScroll.height - updateScrollBox.height - 1) updateScrollBox.y = updateScroll.height - updateScrollBox.height - 1
                else updateScrollBox.y = mouseY + 1
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
