import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import QtQuick.Controls.Material

import com.basilevs.multimedia

Window {
    id: root
    visible: true
    title: "Client"
    width: 800
    height: 450

    Material.theme: Material.Light

    /*width: Style.screenWidth
    height: Style.screenHeigth

    onWidthChanged:{
        Style.calculateRatio(root.width, root.height)
    }*/

    Client {
        id: client
        Component.onCompleted: {
            client.connectToHost("ws://192.168.0.6:41419")
        }
    }

    GridLayout {
        anchors.fill: parent
        columns: root.width < root.height ? 1 : 2
        rows: 3 - columns
        columnSpacing: 0
        rowSpacing: 0

        Video {
            id: videoItem
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 5
            Layout.preferredWidth: 5

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    videoItem.source = "tcp://192.168.0.6:5000"
                    videoItem.play()
                }
            }
        }

        Rectangle {
            color: Material.frameColor
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 3
            Layout.preferredWidth: 3

            GridLayout {
                anchors.fill: parent
                columns: root.width < root.height ? 2 : 1
                rows: 3 - columns
                columnSpacing: 0
                rowSpacing: 0

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.margins: 10

                    Button {
                        Layout.fillWidth: true
                        text: qsTr("Captures")
                    }

                    Button {
                        Layout.fillWidth: true
                        text: qsTr("Settings")
                    }

                    Row {
                        RecordButton {

                        }

                        Label {
                            leftPadding: 10
                            height: parent.height
                            verticalAlignment: Text.AlignVCenter
                            text: "0:00.0"
                        }
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: false
                    //height: width
                    //anchors.fill: parent
                    rows: 3
                    columns: 3
                    Layout.margins: 10
                    rowSpacing: 0
                    columnSpacing: 0

                    Item { Layout.fillWidth: true; Layout.fillHeight: true }
                    RoundButton {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: "^"
                        onClicked: client.sendRotateCmd(Client.DIR_UP)
                    }
                    Item { Layout.fillWidth: true; Layout.fillHeight: true }

                    RoundButton {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: "<"
                        onClicked: client.sendRotateCmd(Client.DIR_LEFT)
                    }
                    Item { Layout.fillWidth: true; Layout.fillHeight: true }

                    RoundButton {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: ">"
                        onClicked: client.sendRotateCmd(Client.DIR_RIGHT)
                    }

                    Item { Layout.fillWidth: true; Layout.fillHeight: true }
                    RoundButton {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: "v"
                        onClicked: client.sendRotateCmd(Client.DIR_DOWN)
                    }
                    Item { Layout.fillWidth: true; Layout.fillHeight: true }
                }
            }
        }
    }
}
