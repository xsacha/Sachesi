import QtQuick 2.2
import Style 1.0

BasicButton {
    id: button

    property alias subtext: subtextValue.text
    property string text
    property url iconSource

    implicitWidth: Math.max(72, backgroundItem.implicitWidth)
    implicitHeight: Math.max(22, backgroundItem.implicitHeight)

    property bool defaultbutton: false
    property string styleHint

    background: StyleItem {
        id: styleitem
        anchors.fill: parent
        elementType: "button"
        sunken: pressed || checked
        raised: !(pressed || checked)
        hover: button.containsMouse
        text: iconSource === "" ? "" : button.text
        hasFocus: button.focus
        hint: button.styleHint

        // If no icon, let the style do the drawing
        activeControl: defaultbutton ? "default" : "f"
    }
    Keys.onSpacePressed:animateClick()
    Text {
        id: subtextValue
        anchors {left: parent.left; leftMargin: 0; top: parent.top; topMargin: parent.height}
        text: ""
        font.pixelSize: config.defaultSubtextSize; font.bold: true; color: "#404040"
    }
}
