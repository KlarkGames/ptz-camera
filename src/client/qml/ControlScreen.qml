import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import QtQuick.Controls.Material
import com.basilevs.multimedia

GridLayout {
    property string ipAddress : undefined

    columns: parent.width < parent.height ? 1 : 2
    rows: 3 - columns
    columnSpacing: 0
    rowSpacing: 0

    Client {
        id: client
        Component.onCompleted: {
            client.connectToHost(ipAddress)
        }

        onIsRecordingChanged: {
            elapsedTime.upd()
        }

        onConnected: {
            videoItem.source = client.streamSource
            videoItem.play()
        }
    }

    Video {
        id: videoItem
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.preferredHeight: 5
        Layout.preferredWidth: 5

        Repeater {
            model: client.isTracking ? client.trackingObjectModel : 0
            delegate: Rectangle {
                x: model.rect.x
                y: model.rect.y
                width: model.rect.width
                height: model.rect.height
                color: "transparent"
                border.color: "red"
                border.width: 3

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        console.debug(model.objectId, model.className)
                    }
                }
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
                        recording: client.isRecording
                        onClicked: {
                            client.sendSetRecordingCmd(!client.isRecording);
                        }
                    }

                    Label {
                        function upd(time) {
                            text = Qt.formatTime(client.getRecElapsedTimeMSecs(), "hh:mm:ss.zzz")
                        }

                        Component.onCompleted: { upd() }

                        id: elapsedTime
                        leftPadding: 10
                        height: parent.height
                        verticalAlignment: Text.AlignVCenter

                        Timer {
                            interval: 97
                            repeat: true
                            triggeredOnStart: true
                            running: client.isRecording
                            onTriggered: {
                                elapsedTime.upd()
                            }
                        }
                    }
                }
            }

            GridLayout {
                Layout.fillWidth: true
                Layout.fillHeight: false
                rows: 3
                columns: 3
                Layout.margins: 10

                rowSpacing: 5
                columnSpacing: 5

                Item { Layout.fillWidth: true; Layout.fillHeight: true }
                Button {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    enabled: !client.isTracking
                    text: "^"
                    onPressed: client.sendRotateCmd(Client.DIR_UP, true)
                    onReleased: client.sendRotateCmd(Client.DIR_UP, false)
                }
                Item { Layout.fillWidth: true; Layout.fillHeight: true }

                Button {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    enabled: !client.isTracking
                    text: "<"
                    onPressed: client.sendRotateCmd(Client.DIR_LEFT, true)
                    onReleased: client.sendRotateCmd(Client.DIR_LEFT, false)
                }

                Button {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    checked: client.isTracking
                    text: "AI"
                    onClicked: client.sendSetTrackingCmd(!client.isTracking)
                }

                Button {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    enabled: !client.isTracking
                    text: ">"
                    onPressed: client.sendRotateCmd(Client.DIR_RIGHT, true)
                    onReleased: client.sendRotateCmd(Client.DIR_RIGHT, false)
                }

                Item { Layout.fillWidth: true; Layout.fillHeight: true }

                Button {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    enabled: !client.isTracking
                    text: "v"
                    onPressed: client.sendRotateCmd(Client.DIR_DOWN, true)
                    onReleased: client.sendRotateCmd(Client.DIR_DOWN, false)
                }
                Item { Layout.fillWidth: true; Layout.fillHeight: true }
            }
        }
    }
}
