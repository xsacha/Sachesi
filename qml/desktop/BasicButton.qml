import QtQuick 1.1

Item {
    id: button

    signal clicked
    property alias pressed: behavior.pressed
    property alias containsMouse: behavior.containsMouse
    property alias checkable: behavior.checkable  // button toggles between checked and !checked
    property alias checked: behavior.checked
    property bool activeFocusOnPress: false

    property Component background: null
    property Item backgroundItem: backgroundLoader.item

    property color textColor: syspal.text;
    property string tooltip

    signal toolTipTriggered

    // implementation

    property string __position: "only"
    implicitWidth: backgroundLoader.item.width
    implicitHeight: backgroundLoader.item.height

    function animateClick() {
        behavior.pressed = true
        behavior.clicked()
        animateClickTimer.start()
    }

    Timer {
        id: animateClickTimer
        interval: 250
        onTriggered: behavior.pressed = false
    }

    Loader {
        id: backgroundLoader
        anchors.fill: parent
        sourceComponent: background
        property alias styledItem: button
        property alias position: button.__position
    }

    ButtonBehavior {
        id: behavior
        anchors.fill: parent
        onClicked: button.clicked()
        onPressedChanged: if (activeFocusOnPress) button.focus = true
        onMouseMoved: {tiptimer.restart()}
        Timer{
            id: tiptimer
            interval:1000
            running:containsMouse && tooltip.length
            onTriggered: button.toolTipTriggered()
        }
    }

    SystemPalette { id: syspal }
}
