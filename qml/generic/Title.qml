import QtQuick 1.1
import "UI" 1.0

Rectangle {
    width: 520
    height: 480
    color: "#868284"
    Config {id:config}

	/*Text {
        font.pixelSize: 22
        text: "SACHESI"
        font.letterSpacing: (parent.width - 280) / 6
        font.weight: Font.DemiBold
        smooth: true
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: (parent.width - 280) / 12
	}*/

    Rectangle {
        anchors { right: parent.right; rightMargin: 1; top: parent.top; topMargin: 1 }
        width: 28; height: 28
        radius: height
        color: p.advanced ? "#999999" : "transparent"
        border.width: 2
        smooth: true
        MouseArea {
            anchors.fill: parent
            onClicked: p.advanced = !p.advanced
        }
    }

    Row {
        id: titleRow
        anchors.verticalCenter: parent.verticalCenter
        anchors.top: parent.top; anchors.topMargin: 7
        spacing: (parent.width - title1.w - title2.w - title3.w - title4.w - title5.w - title6.w) / (p.advanced ? 5 : 3)

        property int curObj: 3;
        TitleObject {
            id: title1
            obj: 0
            text: "Extract"
            property int w: visible ? width : 0
        }
        TitleObject {
            visible: p.advanced && p.hasBootAccess
            id: title2
            obj: 1
            text: "Tools"
            property int w: visible ? width : 0
        }
        TitleObject {
            visible: p.advanced && p.hasBootAccess
            id: title3
            obj: 2
            text: "Boot"
            property int w: visible ? width : 0
        }
        TitleObject {
            id: title4
            obj: 3
            text: "Search"
            property int w: visible ? width : 0
        }
        TitleObject {
            id: title5
            obj: 4
            text: "Backup"
            property int w: visible ? width : 0
        }
        TitleObject {
            id: title6
            obj: 5
            text: "Install"
            property int w: visible ? width : 0
        }
    }
    Loader {
        visible: titleRow.curObj == 0
        anchors.top: parent.top
        anchors.topMargin: 15 + config.notificationFontSize
        width: parent.width;
        height: parent.height - 5 - config.notificationFontSize;
        source: "extract.qml"
    }
    Loader {
        visible: titleRow.curObj == 1
        anchors.top: parent.top
        anchors.topMargin: 15 + config.notificationFontSize
        width: parent.width;
        height: parent.height - 5 - config.notificationFontSize;
        source: p.hasBootAccess ? "downloader.qml" : ""
    }
    Loader {
        visible: titleRow.curObj == 2
        anchors.top: parent.top
        anchors.topMargin: 15 + config.notificationFontSize
        width: parent.width;
        height: parent.height - 5 - config.notificationFontSize;
        source: p.hasBootAccess ? "boot.qml" : ""
    }
    Loader {
        visible: titleRow.curObj == 3
        anchors.top: parent.top
        anchors.topMargin: 15 + config.notificationFontSize
        width: parent.width;
        height: parent.height - 5 - config.notificationFontSize;
		source: "main.qml"
    }
    Loader {
        visible: titleRow.curObj == 4 && (i.knownBattery > -1 && !i.wrongPassBlock)
        anchors.top: parent.top
        anchors.topMargin: 15 + config.notificationFontSize
        width: parent.width;
        height: parent.height - 5 - config.notificationFontSize;
        source: "backup.qml"
    }
    Loader {
        visible: titleRow.curObj == 5 && (i.knownBattery > -1 && !i.wrongPassBlock)
        anchors.top: parent.top
        anchors.topMargin: 15 + config.notificationFontSize
        width: parent.width;
        height: parent.height - 5 - config.notificationFontSize;
        source: "installer.qml"
    }
    Loader {
        visible: titleRow.curObj >= 4 && (i.knownBattery <= -1 || i.wrongPassBlock)
        anchors.top: parent.top
        anchors.topMargin: 15 + config.notificationFontSize
        width: parent.width;
        height: parent.height - 5 - config.notificationFontSize;
        source: "usbconnect.qml"
	}
	/*Rectangle {
		visible: !hasDonated
        width: parent.width; height: 30
        anchors.bottom: parent.bottom
        color: "black"
        Text {
            anchors.centerIn: parent
            text: "If you enjoy using this tool. Please consider donating."
            font.pixelSize: 14
            color: "yellow"
            MouseArea {
                anchors.fill: parent;
                onClicked: {
                    hasDonated = true
                    Qt.openUrlExternally("https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=xsacha@gmail.com&lc=AU&item_name=Time+and+effort&item_number=xsacha&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHosted")
                }
            }
        }
	}*/
}
