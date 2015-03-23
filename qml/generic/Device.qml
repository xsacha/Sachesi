import QtQuick 2.2
import QtQuick.Controls 1.1


Item {
    id: main
    TabView {
        id: deviceSubTabs
        anchors.fill: parent
        frameVisible: count > 1
        tabsVisible: count > 1

        // Placeholder. Todo: Add and Remove on wifi device from C++ Signal.
        // Somehow get the ip in the title too.
        //Component.onCompleted: deviceSubTabs.addTab(qsTr("Wifi"), deviceSubComponent )

        // Permanent tab
        Tab {
            title: qsTr("USB") + translator.lang + ((i.device === null || i.device.battery < 0) ? (" [" + i.ip + "]") : "")
            DeviceSub { anchors.fill: parent }
        }
    }
    Component {
        id: deviceSubComponent
        DeviceSub { anchors.fill: parent }
    }
}
