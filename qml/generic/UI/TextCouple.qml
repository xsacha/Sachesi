import QtQuick 2.2
import QtQuick.Controls 1.1

Item {
    id: itemroot
    property string type
    property bool large: true
    property alias value: textValue.text
    property alias subtext: subtextValue.text
    property alias thisid: textValue
    property alias readOnly: textValue.readOnly
    property alias textColor: textValue.textColor
    property variant before: itemroot
    property variant after: itemroot
    property alias typeOffset: typeText.right
    property alias passMode: textValue.echoMode
    property alias restrictions: textValue.inputMethodHints
    property alias maxLength: textValue.maximumLength
    signal clicked();
    signal upArrow();
    signal downArrow();
    height: config.defaultButtonTextSize * 1.4; width: large ? config.defaultButtonTextSize * 18 : config.defaultButtonTextSize * 7;
    Label {
        id: typeText
        anchors {left: parent.left; leftMargin: 10}
        text: type
        font.bold: true
    }
    Label {
        id: subtextValue
        anchors {left: parent.left; leftMargin: 15; top: parent.top; topMargin: parent.height - 2}
        text: ""
    }
    TextField {
            id: textValue
            width: large ? (config.defaultSubtextSize + 1) * 9 : (config.defaultSubtextSize) * 3
            anchors {left: parent.left; leftMargin: itemroot.width / 2 - (large ? 40 : -10); verticalCenter: parent.verticalCenter}
            height: itemroot.height;
            text: value
            onTextChanged: if (!large && text.length > 4) text = text.substring(0,4)
            KeyNavigation.priority: KeyNavigation.BeforeItem
            KeyNavigation.tab: after
            KeyNavigation.backtab: before
            onAccepted: itemrot.clicked()
            Keys.onUpPressed: { itemroot.upArrow()}
            Keys.onDownPressed: { itemroot.downArrow()}
    }
}
