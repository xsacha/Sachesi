// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import BackupTools 1.0

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
    Column {
        anchors { top: parent.top; topMargin: 20; left: parent.left; leftMargin: 20 }
        spacing: 20
        Column {
            spacing: 10
            Text {
                text: "Options"
                font.pixelSize: 14
                font.bold: true
            }
            Row {
                id: options
                property int value: fullSelect.checked ? 7 : (settingsSelect.checked * 1 + mediaSelect.checked * 2 + appsSelect.checked * 4)
                spacing: 20
                SelectionText {
                    id: settingsSelect
                    text: "Settings"
                    checked: true
                }
                SelectionText {
                    id: mediaSelect
                    text: "Media"
                }
                SelectionText {
                    id: appsSelect
                    text: "Apps"
                }
                SelectionText {
                    id: fullSelect
                    text: "Full"
                    onSelected: {
                        settingsSelect.enabled = !checked
                        mediaSelect.enabled = !checked
                        appsSelect.enabled = !checked
                    }
                }
            }
        }

        Row {
            spacing: 20
            RoundButton {
                text: "Create backup"
                enabled: !i.backing && !i.restoring && options.value != 0
                mouse.onClicked: i.selectBackup(options.value)
            }
            RoundButton {
                text: "Restore backup"
                enabled: !i.backing && !i.restoring && options.value != 0
                mouse.onClicked: i.selectRestore(options.value)
            }
        }
    }
    Rectangle {
        visible: i.backing || i.restoring
        anchors {bottom: parent.bottom; bottomMargin: 10; horizontalCenter: parent.horizontalCenter }
        height: 60; width: parent.width - 80; radius: 8
        z: 5;
        color: "gray"
        opacity: 0.95
        Column {
            anchors {verticalCenter: parent.verticalCenter; left: parent.left; leftMargin: parent.width / 2 - 150 }
            Text {
                font.pixelSize: 20
                text: (i.backing ? "Creating" : "Restoring") + " Backup (" + i.backProgress + "%)";
            }
            Text {
                id: splitText
                font.pixelSize: 20
                text: "";
                property string statusText: i.backStatus + " (" + i.backCurProgress + "%)";
            }
        }
        Timer {
            running: parent.visible
            repeat: true
            triggeredOnStart: true
            interval: 400
            function incDots() {
                if ( typeof incDots.dots === 'undefined' )
                    incDots.dots = 0;
                splitText.text = splitText.statusText
                for (var i = 0; i < incDots.dots; i++)
                    splitText.text += ".";
                if (incDots.dots === 3)
                    incDots.dots = 0;
                incDots.dots++;
            }
            onTriggered: incDots();
        }
    }
    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom;
        anchors.bottomMargin: 100
        text: "Restore Coming Soon"
        font.pixelSize: 30
    }
}
