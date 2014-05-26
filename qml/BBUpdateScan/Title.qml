// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Rectangle {
    width: 480
    height: 380
    color: "#868284"
    Text {
        font.pixelSize: 22
        text: "SACHESI"
        font.letterSpacing: (parent.width - 280) / 6
        font.weight: Font.DemiBold
        smooth: true
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: (parent.width - 280) / 12
    }

    Row {
        id: titleRow
        anchors.verticalCenter: parent.verticalCenter
        anchors.top: parent.top; anchors.topMargin: 25
        spacing: (parent.width - 432) / 4

        property int curObj: 2;
        TitleObject {
            obj: 0
            text: "Extractor"
        }
        TitleObject {
            obj: 1
            text: "Exploits"
        }
        TitleObject {
            obj: 2
            text: "Searcher"
        }
        TitleObject {
            obj: 3
            text: "Backup"
        }
        TitleObject {
            obj: 4
            text: "Installer"
        }
    }
    Loader {
        visible: titleRow.curObj == 0
        anchors.bottom: parent.bottom
        anchors.bottomMargin: -10
        width: parent.width;
        height: parent.height - 40;
        source: "tools.qml"
    }
    Loader {
        visible: titleRow.curObj == 1
        anchors.bottom: parent.bottom
        anchors.bottomMargin: -10
        width: parent.width;
        height: parent.height - 40;
        source: "downloader.qml"
    }
    Loader {
        visible: titleRow.curObj == 2
        anchors.bottom: parent.bottom
        anchors.bottomMargin: -10
        width: parent.width;
        height: parent.height - 40;
        source: "main.qml"
    }
    Loader {
        visible: titleRow.curObj == 3 && i.knownBattery > -1
        anchors.bottom: parent.bottom
        anchors.bottomMargin: -10
        width: parent.width;
        height: parent.height - 40;
        source: "backup.qml"
    }
    Loader {
        visible: titleRow.curObj == 4 && i.knownBattery > -1
        anchors.bottom: parent.bottom
        anchors.bottomMargin: -10
        width: parent.width;
        height: parent.height - 40;
        source: "installer.qml"
    }
    Loader {
        visible: titleRow.curObj >= 3 && i.knownBattery <= -1
        anchors.bottom: parent.bottom
        anchors.bottomMargin: -10
        width: parent.width;
        height: parent.height - 40;
        source: "usbconnect.qml"
    }
}
