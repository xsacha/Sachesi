// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

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

    Rectangle {
        visible: p.splitting
        anchors {bottom: parent.bottom; bottomMargin: 10; horizontalCenter: parent.horizontalCenter }
        height: 80; width: parent.width - 40; radius: 8
        z: 5;
        color: "gray"
        opacity: 0.95
        Text {
            id: splitText
            property string typeText: ""
            property string statusText: typeText + ((p.splitting == 5) ? "" : "(" + p.splitProgress + "%)");
            font.pixelSize: 26
            anchors {top: parent.top; topMargin: 10; left: parent.left; leftMargin: parent.width * 0.5 - statusText.length * 7 }
            Timer {
                property int splitVal: p.splitting
                onSplitValChanged: {
                    if (p.splitting == 1)
                        splitText.typeText = "Splitting Autoloader ";
                    else if (p.splitting == 2)
                        splitText.typeText = "Combining Autoloader ";
                    else if (p.splitting == 3)
                        splitText.typeText = "Extracting Image ";
                    else if (p.splitting == 4)
                        splitText.typeText = "Extracting Apps ";
                    else if (p.splitting == 5)
                        splitText.typeText = "Fetching required files ";
                }
                running: splitVal
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
        RoundButton {
            z: 6;
            visible: p.splitting == 2
            anchors {bottom: parent.bottom; bottomMargin: 10; horizontalCenter: parent.horizontalCenter}
            text: "Cancel";
            mouse.onClicked: p.abortSplit();
        }
    }

    Column {
        spacing: (parent.height - 300) / 5
        anchors { left: parent.left; leftMargin: 20; top: parent.top; topMargin: 20 }
        Text {
            text: "Autoloader Tools"
            font.pixelSize: 14
            font.bold: true
        }
        Row {
            spacing: 20
            RoundButton {
                text: "Split";
                subtext: "Split/combine signed images from/to autoloaders"
                enabled: !p.splitting
                mouse.onClicked: if (!p.splitting) p.splitAutoloader();
            }
            RoundButton {
                text: "Combine";
                enabled: !p.splitting
                mouse.onClicked: if (!p.splitting) p.combineAutoloader();
            }
        }
        Rectangle { color: "transparent"; width: 1; height: 20 }
        Text {
            text: "Signed Image Tools"
            font.pixelSize: 14
            font.bold: true
        }
        Column {
            spacing: 30

            Row {
                spacing: 20
                RoundButton {
                    text: "Dump All"
                    subtext: "Extract images and dump contents"
                    enabled: !p.splitting
                    mouse.onClicked: if (!p.splitting) p.extractImage(0);
                }
            }
            Row {
                spacing: 20
                RoundButton {
                    text: "Extract OS"
                    subtext: "Extract OS image"
                    enabled: !p.splitting
                    mouse.onClicked: if (!p.splitting) p.extractImage(1);
                }

                RoundButton {
                    text: "Extract Apps"
                    subtext: "Extract all bar archives"
                    enabled: !p.splitting
                    mouse.onClicked: if (!p.splitting) p.extractImage(2);
                }
            }
        }
    }
}
