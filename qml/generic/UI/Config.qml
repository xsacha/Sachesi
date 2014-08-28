//import QtQuick 2.2
import QtQuick 2.2

Item {
    visible: false
    SystemPalette {id: pal}
	property string windowColor: "#EEEEEE" //pal.window
    property string textColor: "black" //pal.windowText
	property string shadowColor: "#AAAAAA" //pal.shadow
	property string topColor: "#C0C0C0" //pal.mid
	property string bottomColor: "#F0F0F0" //pal.midlight
	property string darkColor: "#A0A0A0" //pal.dark

    //property int scale: 1 + (parent.width - 520) / 800

    property int defaultFontSize: 11 + 6.5 //+ parent.width / 80
    property int defaultButtonTextSize: 9 + 6.5 //+ parent.width / 80
    property int defaultSubtextSize: 7 + 6.5 //+ parent.width / 80
    property int notificationFontSize: 22 + 4//+ parent.width / 140

    property int defaultWidth: 480
    property int defaultHeight: 360
}
