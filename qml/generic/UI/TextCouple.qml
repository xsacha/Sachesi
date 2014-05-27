import QtQuick 1.1

Item {
    id: itemroot
    property string type
    property bool large: true
    property alias value: textValue.text
    property alias subtext: subtextValue.text
    property alias thisid: textValue
    property alias readOnly: textValue.readOnly
    property alias textColor: textValue.color
    property variant before: itemroot
    property variant after: itemroot
    property alias typeOffset: typeText.right
    property alias passMode: textValue.echoMode
    signal clicked();
    signal upArrow();
    signal downArrow();
    height: config.defaultButtonTextSize * 1.4; width: large ? config.defaultButtonTextSize * 14 : config.defaultButtonTextSize * 7;
    Text {
        id: typeText
        anchors {left: parent.left; leftMargin: 10}
        text: type
        font.pixelSize: config.defaultButtonTextSize; font.bold: true
    }
    Text {
        id: subtextValue
        anchors {left: parent.left; leftMargin: 15; top: parent.top; topMargin: parent.height - 2}
        text: ""
        font.pixelSize: config.defaultSubtextSize; font.bold: true; color: "#404040"
    }
    Rectangle {
        anchors {left: parent.left; leftMargin: itemroot.width / 2 - (large ? 10 : -10)}
        height: itemroot.height; width: large ? (config.defaultSubtextSize + 1) * 7 : (config.defaultSubtextSize) * 3
        radius: 2

        TextInput {
            id: textValue
            width: parent.width
            anchors {left: parent.left; leftMargin: 5; verticalCenter: parent.verticalCenter }
            font.pixelSize: config.defaultSubtextSize + 1
            text: value
            onTextChanged: if (!large && text.length > 4) text = text.substring(0,4)
            KeyNavigation.priority: KeyNavigation.BeforeItem
            KeyNavigation.tab: after
            KeyNavigation.backtab: before
            Keys.onReturnPressed: { itemroot.clicked()}
            Keys.onEnterPressed: { itemroot.clicked()}
            selectByMouse: true
            Keys.onUpPressed: { itemroot.upArrow()}
            Keys.onDownPressed: { itemroot.downArrow()}
        }
    }
}
