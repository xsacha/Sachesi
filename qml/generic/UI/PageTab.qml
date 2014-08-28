import QtQuick 2.2

Rectangle {
    Config {id: config}
    width: 520
    height: 410
    radius: 6
    border {width: 2; color: config.shadowColor }
	Image {
        source: "../section_background_pattern_odd.png"
		fillMode: Image.Tile
		anchors.fill: parent
	}/*
    gradient: Gradient {
        GradientStop { position: 0.0; color: config.topColor }
        GradientStop { position: 1.0; color: config.bottomColor }
	}*/
}
