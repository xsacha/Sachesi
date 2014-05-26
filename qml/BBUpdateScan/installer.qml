// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import Drop 1.0
import AppLibrary 1.0

Rectangle {
    id: main
    width: 480
    height: 340
    radius: 8
    SystemPalette {id: pal}
    border.width: 2
    border.color: pal.shadow
    gradient: Gradient {
        GradientStop { position: 0.0; color: pal.mid }
        GradientStop { position: 1.0; color: pal.midlight }
    }

    property string newLine: i.newLine
    property string details: ""
    property string lasterror: "\n"
    property bool connected: i.knownBattery > -1
    onNewLineChanged: details += i.newLine

    Text {
        anchors { bottom: parent.bottom; left: parent.left; leftMargin: 20; bottomMargin: 12 }
        text: "<b>[</b>USB" + " ("+i.knownBattery+"%)<b>]</b>  " + "  <b>[</b>OS" + i.knownOS + "<b>]</b>";
        font.pixelSize: 12
    }

    Text {
        anchors { bottom: parent.bottom; right: parent.right; rightMargin: 20; bottomMargin: 12 }
        z: 2
        property string hwid: i.knownHW
        property string dName: ""
        onHwidChanged: { switch(hwid.toUpperCase().slice(-4)) {
            case "A106":
                dName = "Playbook";
                if (hwid.charAt(1) != '6') dName += " (4G)";
                break;
            case "2307": dName = "Dev Alpha A"; break;
            case "2607": dName = "Z10 STL-1"; break;
            case "240A": {
                dName = "Z10";
                switch (hwid.charAt(1))
                {
                case '7': dName += " STL-2"; break;
                case '5': dName += " STL-3"; break;
                case '4': dName += " STL-4"; break;
                }
            }
            break;
            case "270A": {
                dName = "Q10"
                switch (hwid.charAt(1))
                {
                case '4': dName += " SQN-1"; break;
                case '5': dName += " SQN-2"; break;
                case '6': dName += " SQN-3"; break;
                case 'c': dName += " SQN-4"; break;
                case '7': dName += " SQN-5"; break;
                case 'd': dName += " DAC"; break;
                }
            }
            break;
            default: dName = hwid; break;
            }
        }
        text: "<b>[</b>" + i.knownName + " ("+dName+")<b>]</b>"
        font.pixelSize: 12
    }

    DropArea {
        anchors.fill: parent
        onFileDrop: { i.install(text); tabs.curObj = 1 }
    }
    Column {
        id: toolsColumn
        anchors {top: parent.top; topMargin: 20; left: parent.left; leftMargin: 20 }
        spacing: 10
        Text {
            id: text1
            text: "Tools"
            font.pixelSize: 14
            font.bold: true
        }
        Row {
            spacing: 20
            RoundButton {
                id: install_files
                text: "Install .BAR(s)"
                mouse.onClicked: { i.selectInstall(); tabs.curObj = 1; }
            }
            RoundButton {
                id: list_files
                text: "List Installed"
                mouse.onClicked: { i.listApps(); tabs.curObj = 1 }
            }
        }
        Text {
            id: helpText
            text: "To install BAR files such as applications or firmware, just Drag and Drop."
            font.pixelSize: 12
        }
    }

    Row {
        id: tabs
        property int curObj: 0
        anchors { top: toolsColumn.bottom; topMargin: 15; left: toolsColumn.left }
        TabObject {
            obj: 0
            text: "Apps"
        }
        TabObject {
            obj: 1
            text: "Log"
        }
    }

    // Log
    Item {
        visible: tabs.curObj == 1
        anchors {top: tabs.bottom; left: tabs.left; }
        height: parent.height - 160 - Math.floor(parent.width / 100); width: parent.width - 40; z: 2;
        Rectangle {
            id: updateMessage
            anchors.left: parent.left
            height: parent.height; width: parent.width - 18;
            color: pal.window
            border.color: pal.shadow
            border.width: 2
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
                    text: details
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
                anchors {left: parent.left; leftMargin: 1 }
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
    }

    // Applications
    Rectangle {
        id: appList
        visible: tabs.curObj == 0
        anchors {top: tabs.bottom; left: tabs.left; }
        height: parent.height - 160; width: parent.width - 40; z: 2;
        color: pal.window
        border.width: 2
        ListView {
            id: appView
            anchors {left: parent.left; leftMargin: 5; bottom: parent.bottom }
            width: parent.width - 10; height: parent.height;
            spacing: 3
            clip: true
            onVisibleChanged: model = i.appList
            delegate: Row {
                width: parent.width
                height: 20
                Image {
                    visible: type === "application"
                    width: 20; height: 20;
                    source: "trash.png"
                    scale: delMouse.pressed ? 0.8 : 1.0
                    Behavior on scale { NumberAnimation { duration: 100 } }
                    MouseArea {
                        id: delMouse
                        anchors.fill: parent
                        //onClicked:
                    }
                }
                Item { visible: type !== "application"; width: 20; height: 20 }
                Text {
                    text: friendlyName
                    font.pixelSize: 12
                    width: parent.width - 100
                    clip: true
                }
                Text {
                    text: version
                    font.pixelSize: 12;
                    width: 100
                    clip: true
                }
            }
        }
    }
}
