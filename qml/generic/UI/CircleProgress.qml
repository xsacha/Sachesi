import QtQuick 2.0
import QtQml 2.1

Canvas {
    id: canvas
    //width: 240
    //height: 240

    property color primaryColor: "darkorange"
    property color secondaryColor: "steelblue"

    property real centerWidth: width / 2
    property real centerHeight: height / 2
    property real radius: Math.min(canvas.width - 8, canvas.height - 8) / 2

    property real minimumValue: 0
    property real maximumValue: 100
    property real currentValue: 0
    // To show on click
    property real overallValue: -1
    property real viewValue: (overallValue > -1 && mouseArea.pressed) ? overallValue : currentValue
    property real curId: 0
    property real maxId: 0

    // this is the angle that splits the circle in two arcs
    // first arc is drawn from 0 radians to angle radians
    // second arc is angle radians to 2*PI radians
    property real angle: (viewValue - minimumValue) / (maximumValue - minimumValue) * 2 * Math.PI

    // we want both circle to start / end at 12 o'clock
    // without this offset we would start / end at 9 o'clock
    property real angleOffset: -Math.PI / 2

    property string text: ""

    signal clicked()

    onPrimaryColorChanged: requestPaint()
    onSecondaryColorChanged: requestPaint()
    onMinimumValueChanged: requestPaint()
    onMaximumValueChanged: requestPaint()
    onHeightChanged: requestPaint();
    // Keeps turning off
    onViewValueChanged: { requestPaint(); antialiasing = true; }

    onPaint: {
        var ctx = getContext("2d");
        ctx.save();

        ctx.clearRect(0, 0, canvas.width, canvas.height);

        // fills the mouse area when pressed
        // the fill color is a lighter version of the
        // secondary color

        if (mouseArea.pressed) {
            ctx.beginPath();
            ctx.lineWidth = 1;
            ctx.fillStyle = Qt.lighter(canvas.primaryColor, 1.25);
            ctx.arc(canvas.centerWidth,
                    canvas.centerHeight,
                    canvas.radius,
                    0,
                    2*Math.PI);
            ctx.fill();
        }

        // First, thinner arc
        // From angle to 2*PI

        ctx.beginPath();
        ctx.lineWidth = 3;
        ctx.strokeStyle = primaryColor;
        ctx.arc(canvas.centerWidth,
                canvas.centerHeight,
                canvas.radius,
                angleOffset + canvas.angle,
                angleOffset + 2*Math.PI);
        ctx.stroke();


        // Second, thicker arc
        // From 0 to angle

        ctx.beginPath();
        ctx.lineWidth = 8;
        ctx.strokeStyle = canvas.secondaryColor;
        ctx.arc(canvas.centerWidth,
                canvas.centerHeight,
                canvas.radius,
                canvas.angleOffset,
                canvas.angleOffset + canvas.angle);
        ctx.stroke();

        ctx.restore();
    }

    Column {
        anchors.centerIn: parent
        spacing: height / 5
        add: Transition {
            NumberAnimation {
                duration: 100
                properties: "y"
                easing.type: Easing.OutBounce
            }
        }

        Text {
            text: canvas.text.substring(0, 35)
            //color: canvas.primaryColor
            anchors.horizontalCenter: parent.horizontalCenter
        }
        // Show count if we are looking at individual percents
        Text {
            visible: !mouseArea.pressed && maxId > 0;
            text: qsTr("%1 of %2").arg(curId).arg(maxId) + translator.lang
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Text {
            text: viewValue + "%"
            font.bold: true
            scale: mouseArea.pressed ? 2.0 : 1.0
            Behavior on scale { NumberAnimation { duration: 100 } }
            color: canvas.secondaryColor
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        onClicked: canvas.clicked();
        onPressedChanged: canvas.requestPaint()
    }
}
