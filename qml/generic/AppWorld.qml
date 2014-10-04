import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

Item {
    id: main
    anchors { fill: parent; leftMargin: 20; topMargin: 20 }
    ColumnLayout {
        anchors.fill: parent
        Layout.fillHeight: true
        Layout.fillWidth: true
        RowLayout {
            Button {
                visible: !appworld.listing
                text:  qsTr("Back")
                onClicked: appworld.listing = true
            }
            Button {
                text:  qsTr("Home")
                onClicked: appworld.showHome()
            }
            TextField {
                id: searchText
                placeholdertext:  qsTr("Search")
                onAccepted: if (searchButton.enabled) searchButton.clicked()
            }
            Button {
                text:  qsTr("Featured")
                onClicked: appworld.searchLocker("featured")
            }
            Button {
                text:  qsTr("Carrier")
                onClicked: appworld.searchLocker("mypreloadeditems")
            }
            Button {
                visible: !blackberry
                text:  qsTr("Cars")
                onClicked: appworld.showCars()
            }

            Button {
                enabled: searchText.length > 0
                id: searchButton
                text:  qsTr("Search")
                onClicked: appworld.search(searchText.text)
            }
        }

        // This is the app detail slide
        ScrollView {
            id: contentView
            Layout.fillHeight: true
            Layout.minimumWidth: main.width
            Layout.maximumWidth: main.width
            visible: !appworld.listing
            ColumnLayout {
                RowLayout {
                    Image {
                        fillMode: Image.Stretch
                        visible: status == Image.Ready
                        source: appworld.contentItem.image == "" ? "" : appworld.contentItem.image + "/X96/png"
                    }

                    Label {
                        text: appworld.contentItem.friendlyName
                        font.pointSize: 12
                    }
                }
                Label {
                    text:  qsTr("by <b><a href=\"#\">" + appworld.contentItem.vendor + "</a></b>")
                    onLinkActivated: appworld.showVendor(appworld.contentItem.vendorId)
                }

                RowLayout {
                    Label {
                        visible: appworld.contentItem.size != 0
                        text:  qsTr("<b>File Bundle</b>: " + appworld.contentItem.name + " <b>Version</b>: " + appworld.contentItem.version + " [" + (appworld.contentItem.size / 1024 / 1024).toFixed(2) + " MB]")
                    }

                    Button {
                        visible: !blackberry
                        text:  qsTr("View")
                        onClicked: Qt.openUrlExternally("http://appworld.blackberry.com/webstore/content/" + appworld.contentItem.id)
                    }

                    Button {
                        visible: blackberry
                        text:  qsTr("Download")
                        // EMBARGO: Exploit not available open-source
                    }
                    Button {
                        visible: !blackberry && appworld.contentItem.size != 0
                        text: appworld.contentItem.price
                        // Edit: Of course, this is wrong ;) They've changed it so I need to login now. We'll do this later
                        // Example: "http://appworld.blackberry.com/ClientAPI/usfdownload?contentid=" + appworld.contentItem.id
                        // Replies:  <error id="30702" type="accounts">Invalid token, please login.</error>
                        onClicked: Qt.openUrlExternally("http://appworld.blackberry.com/webstore/file/" + appworld.contentItem.fileId)
                    }
                }

                Label {
                    Layout.maximumWidth: contentView.width * 0.75
                    wrapMode: Text.Wrap
                    text: appworld.contentItem.description
                }
                ListView {
                    // Force entire screen width otherwise it will be constrained by Layout
                    Layout.minimumWidth: contentView.width - 40
                    // Hardcoded to the width set in image source
                    height: 256
                    spacing: 10
                    orientation: ListView.Horizontal
                    model: appworld.contentItem.screenshots
                    delegate: Image {
                        source: modelData + "/X256/png"
                        Behavior on scale { SpringAnimation { spring: 2; damping: 0.2 } }
                        scale: 0.0
                        onStatusChanged: if (status == Image.Ready) scale = 1.0
                    }
                }
            }
        }

        // This is the grid of apps slide
        ScrollView {
            visible: appworld.listing
            Layout.fillHeight: true
            Layout.minimumWidth: main.width
            Layout.maximumWidth: main.width
            GridView {
                Rectangle {
                    anchors.fill: parent
                    color: "black"
                    z: -1
                }
                id: view
                anchors.fill: parent
                model: appworld.appList
                // 5 is a special number that AppWorld makes the app count divisible by. If in doubt, read it directly from the XML.
                cellWidth: (main.width - 20) / 9 //размер иконок
                cellHeight: cellWidth
                footer: Label { visible: false; text:  qsTr("."); } // Spacer

                delegate: Item {
                    id: item
                    width: view.cellWidth
                    height: view.cellHeight
                    BusyIndicator {
                        running: !imageItem.visible
                        visible: running
                        anchors.fill: parent
                    }
                    Image {
                        id: imageItem
                        visible: status == Image.Ready && image != ""
                        anchors { fill: parent; margins: 30 }
                        height: item.height - textItem.implicitHeight
                        width: item.width
                        fillMode: Image.PreserveAspectFit
                        source: image == "" ? "" : image + "/128X/png" //качество иконок
                        Behavior on scale { SpringAnimation { spring: 2; damping: 0.2 } }
                        scale: 0.0
                        onStatusChanged: if (status == Image.Ready) scale = 1.0
                        MouseArea {
                            anchors.fill: parent
                            onClicked: type == "category" ? appworld.searchMore(id, true) : appworld.showContentItem(id)
                        }
                    }
                    // TODO: Probably make this two strings so we can elide properly
                    Label {
                        id: textItem
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text:  qsTr("<b>" + friendlyName + (vendor == "" ? "" : "</b><br><a href=\"#\">" + vendor + "</a>"))
                        color: "white"
                        linkColor: "lightblue"
                        maximumLineCount: 2
                        clip: true
                        onLinkActivated: appworld.showVendor(vendorId)
                        anchors.top: imageItem.visible ? imageItem.bottom : item.top
                        height: imageItem.visible ? implicitHeight : item.height
                        width: item.width
                        elide: Text.ElideRight
                    }
                }
            }
        }
        RowLayout {
            visible: appworld.listing && appworld.more.length > 0
            Repeater {
                model: appworld.more
                Button {
                    text: modelData.split(",")[0].replace("Apps & Games", "")
                    onClicked: appworld.searchMore(modelData.split(",")[1])
                }
            }
        }
        Text {
            text:  qsTr("Options")
            font.pointSize: 14
            font.bold: true
        }
        RowLayout {
            Label {
                text:  qsTr("Server")
                font.bold: true
            }
            ComboBox {
                currentIndex: appworld.server
                model: ["Production", "Enterprise", "Eval"]
                onCurrentIndexChanged: appworld.server = currentIndex
            }
            Label {
                text:  qsTr("<b>Model</b>: Z10-STL100-2")
            }
            Label {
                text:  qsTr("OS")
                font.bold: true
            }
            ComboBox {
                currentIndex: appworld.osVer
                model: ["Latest", "All"]
            }
        }
    }
}
