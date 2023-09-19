import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

ScrollView {
    id: root

    property alias model: listView.model
    property alias delegate: listView.delegate
    property alias currentIndex: listView.currentIndex
    property alias currentItem: listView.currentItem

    signal clicked(var modelIndex)

    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

    background: Rectangle {
        id: bgRect
        border.width: 1
        border.color: Material.frameColor
        color: Material.backgroundColor
    }

    ListView {
        id: listView
        anchors.fill: parent
        currentIndex: -1
        clip: true

        delegate: ItemDelegate {
            width: root.width
            text: model.hostname
            onClicked: root.clicked(root.model.index(index, 0))
        }
    }
}
