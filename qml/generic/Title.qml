import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Window 2.1
import Qt.labs.settings 1.0
import "UI" 1.0

ApplicationWindow {
    id: window
    title: "Sachesi " + version
    width: 820
    height: 680
    minimumHeight: 540
    minimumWidth: 640
    function isNewer(newV, oldV){

        var result=false;

        if(typeof newV !== 'object'){ newV=newV.toString().split('.'); }
        if(typeof oldV !== 'object'){ oldV=oldV.toString().split('.'); }

        for(var i=0; i<(Math.max(newV.length, oldV.length)); i++){
            if(newV[i] == undefined){ newV[i]=0; }
            if(oldV[i] == undefined){ oldV[i]=0; }

            if(Number(newV[i])>Number(oldV[i])){
                result=true;
                break;
            }
            if(newV[i] != oldV[i]){
                break;
            }
        }
        return(result);
    }

    Component.onCompleted: {
        var http = new XMLHttpRequest()
        var url = "https://raw.githubusercontent.com/xsacha/Sachesi/master/Sachesi.pro";
        http.open("GET", url, true);
        http.send(null)
        http.onreadystatechange = function() {
            if(http.readyState == 4 && http.status == 200) {
                var array = http.responseText.split('\n');
                array = array.filter(function(e){return e});
                for (var i = 1; i < 10; i++) {
                    if (array[i].substring(0, 7) === "VERSION") {
                        var newVer = array[i].split(' ').pop()
                        if (isNewer(newVer, version)) {
                            console.log("NEW VERSION FOUND. DO STUFF HERE")
                        }

                        break;
                    }
                }
            }
        }
    }

    Settings {
        id: settings
        property alias x: window.x
        property alias y: window.y
        property alias width: window.width
        property alias height: window.height
        property url installFolder
        property url backupFolder
        property bool advanced: false
        property bool nativelang: true
    }

    Label {
        visible: !mobile
        id: title
        font.pointSize: 18
        text: "SACHESI"
        font.letterSpacing: (parent.width / 2) / text.length
        font.weight: Font.DemiBold
        smooth: true
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: font.letterSpacing / 2
    }

    Button {
        anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: 1 }
        height: title.height + 7; width: height
        checkable: true
        checked: settings.advanced
        onClicked: settings.advanced = !settings.advanced
        text: checked ? "+H+" : "H"
        tooltip: qsTr("Advanced") + translator.lang
    }

    ComboBox {
        model: [ Qt.locale().nativeLanguageName, "English" ]
        anchors { right: parent.right; top: parent.top; topMargin: 1 }
        visible: translator.exists
        currentIndex: settings.nativelang ? 0 : 1

        onCurrentIndexChanged: {
            if (currentIndex === 0) {
                translator.load();
                settings.nativelang = true;
            } else {
                translator.remove();
                settings.nativelang = false;
            }
        }
    }

    TabView {
        id: titleRow
        width: parent.width
        // Qt5.2 bug requires timer to redraw layout correctly
        Timer {
            interval: 10 // Any number works
            running: true
            onTriggered: titleRow.currentIndex = 4
        }

        anchors {top: title.bottom; bottom: parent.bottom }

        Tab {
            title: qsTr("Device") + translator.lang
            Device { anchors.fill: parent }
        }
        Tab {
            title: qsTr("Extract") + translator.lang
            Extract { anchors.fill: parent }
        }
        Tab {
            title: qsTr("Search") + translator.lang
            Search { anchors.fill: parent }
        }
        Tab {
            title: qsTr("Backup") + translator.lang
            Backup { anchors.fill: parent }
        }
        Tab {
            title: qsTr("Install") + translator.lang
            Installer { anchors.fill: parent }
        }

        USBConnect { anchors.fill: parent }
    }
}
