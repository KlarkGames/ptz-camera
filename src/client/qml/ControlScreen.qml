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
        mediaPlayer: player

        Component.onCompleted: {
            client.connectToHost(ipAddress)
        }

        onIsRecordingChanged: {
            elapsedTime.upd()
        }

        /*onConnected: {
            mediaPlayer.source = client.streamSource
            mediaPlayer.play()
        }*/
    }

    MediaPlayer {
        id: player
        videoOutput: videoItem

        /*onMediaStatusChanged: {
            if (mediaStatus == MediaPlayer.LoadingMedia) {

            }
        }*/
    }

    VideoOutput {
        id: videoItem
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.preferredHeight: 5
        Layout.preferredWidth: 5

        AnimatedImage {
            anchors.centerIn: parent
            width: 50
            height: 50
            source: "qrc:/res/loading.gif"
            visible: player.mediaStatus == MediaPlayer.LoadingMedia
        }

        Repeater {
            model: client.isTracking ? client.trackingObjectModel : 0
            delegate: Rectangle {
                Text {
                    color: "red"
                    font.pixelSize: 12
                    text: model.className
                    x: 0
                    y: -15
                }

                x: model.rect.x / videoItem.sourceRect.width * videoItem.contentRect.width + videoItem.contentRect.x
                y: model.rect.y / videoItem.sourceRect.height * videoItem.contentRect.height + videoItem.contentRect.y
                width: model.rect.width / videoItem.sourceRect.width * videoItem.contentRect.width
                height: model.rect.height / videoItem.sourceRect.height * videoItem.contentRect.height
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
