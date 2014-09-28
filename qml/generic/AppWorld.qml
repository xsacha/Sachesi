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
                text: "Home"
                onClicked: appworld.showFeatured()
            }
            TextField {
                id: searchText
                placeholderText: "Search"
                onAccepted: if (searchButton.enabled) searchButton.clicked()
            }
            Button {
                enabled: searchText.length > 0
                id: searchButton
                text: "Search"
                onClicked: appworld.search(searchText.text)
            }
        }
        ScrollView {
            id: contentView
            Layout.fillHeight: true
            Layout.minimumWidth: main.width
            Layout.maximumWidth: main.width
            visible: !appworld.listing
            ColumnLayout {
                RowLayout {
                    Image {
                        source: appworld.contentItem.image == "" ? "" : appworld.contentItem.image + "/X96/png"
                    }

                    Label {
                        text: appworld.contentItem.friendlyName
                        font.pointSize: 24
                    }
                }
                Label {
                    text: "by <b>" + appworld.contentItem.vendor + "</b>"
                }

                RowLayout {
                    Label {
                        visible: appworld.contentItem.size != 0
                        text: "<b>File Bundle</b>: " + appworld.contentItem.name + " <b>Version</b>: " + appworld.contentItem.version + " [" + (appworld.contentItem.size / 1024 / 1024).toFixed(2) + " MB]"
                    }

                    Button {
                        text: "View"
                        onClicked: Qt.openUrlExternally("http://appworld.blackberry.com/webstore/content/" + appworld.contentItem.id)
                    }
                    Button {
                        //enabled: false
                        text: "Download"
                        // Edit: Of course, this is wrong ;)
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

        ScrollView {
            visible: appworld.listing
            Layout.fillHeight: true
            Layout.fillWidth: true
            GridView {
                id: view
                anchors.fill: parent
                model: appworld.appList
                cellWidth: 200
                cellHeight: 200

                delegate: Item {
                    id: item
                    width: view.cellWidth
                    height: view.cellHeight
                    Image {
                        id: imageItem
                        visible: status == Image.Ready
                        anchors { fill: parent; margins: 20 }
                        height: item.height - textItem.implicitHeight
                        width: item.width
                        fillMode: Image.PreserveAspectFit
                        source: image == "" ? "" : image + "/128X/png"
                        Behavior on scale { SpringAnimation { spring: 2; damping: 0.2 } }
                        scale: 0.0
                        onStatusChanged: if (status == Image.Ready) scale = 1.0
                        MouseArea {
                            anchors.fill: parent
                            onClicked: appworld.showContentItem(id)
                        }
                    }
                    Label {
                        id: textItem
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text: friendlyName
                        anchors.top: imageItem.visible ? imageItem.bottom : item.top
                        height: imageItem.visible ? implicitHeight : item.height
                        width: item.width
                        elide: Text.ElideRight
                    }
                }
            }
        }
        Text {
            text: "Options"
            font.pointSize: 14
            font.bold: true
        }
        RowLayout {
            Label {
                text: "Server"
                font.bold: true
            }
            ComboBox {
                currentIndex: appworld.server
                model: ["Production", "Eval"]
                onCurrentIndexChanged: appworld.server = currentIndex
            }
            Label {
                text: "<b>Model</b>: Passport"
            }
            Label {
                text: "<b>OS</b>: 10.9.0"
            }
        }
    }
}
