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
    }

    MediaPlayer {
        id: player
        videoOutput: videoItem
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

        Rectangle {
            visible: client.isTracking
            x: parseInt(videoItem.sourceRect.width / 2 - videoItem.sourceRect.width * client.horizontalBorder / 2, 10) / videoItem.sourceRect.width * videoItem.contentRect.width + videoItem.contentRect.x
            y: parseInt(videoItem.sourceRect.height / 2 - videoItem.sourceRect.height * client.verticalBorder / 2, 10) / videoItem.sourceRect.height * videoItem.contentRect.height + videoItem.contentRect.y
            width: parseInt(videoItem.sourceRect.width * client.horizontalBorder, 10) / videoItem.sourceRect.width * videoItem.contentRect.width
            height: parseInt(videoItem.sourceRect.height * client.verticalBorder, 10) / videoItem.sourceRect.height * videoItem.contentRect.height
            color: "transparent"
            border.color: "blue"
            border.width: 3

            Behavior on x {
                NumberAnimation {
                    duration: 80
                    easing.type: Easing.InOutQuad
                }
            }

            Behavior on y {
                NumberAnimation {
                    duration: 80
                    easing.type: Easing.InOutQuad
                }
            }

            Behavior on width {
                NumberAnimation {
                    duration: 80
                    easing.type: Easing.InOutQuad
                }
            }

            Behavior on height {
                NumberAnimation {
                    duration: 80
                    easing.type: Easing.InOutQuad
                }
            }
        }

        Repeater {
            model: (player.playbackState == MediaPlayer.PlayingState) && client.isTracking ? client.trackingObjectModel : 0
            delegate: Rectangle {
                x: model.rect.x / videoItem.sourceRect.width * videoItem.contentRect.width + videoItem.contentRect.x
                y: model.rect.y / videoItem.sourceRect.height * videoItem.contentRect.height + videoItem.contentRect.y
                width: model.rect.width / videoItem.sourceRect.width * videoItem.contentRect.width
                height: model.rect.height / videoItem.sourceRect.height * videoItem.contentRect.height
                color: "transparent"
                border.color: (model.objectId == client.trackingObjectModel.trackingId) ? "yellow" : "red"
                border.width: 3

                Behavior on x {
                    NumberAnimation {
                        duration: 80
                        easing.type: Easing.InOutQuad
                    }
                }

                Behavior on y {
                    NumberAnimation {
                        duration: 80
                        easing.type: Easing.InOutQuad
                    }
                }

                Behavior on width {
                    NumberAnimation {
                        duration: 80
                        easing.type: Easing.InOutQuad
                    }
                }

                Behavior on height {
                    NumberAnimation {
                        duration: 80
                        easing.type: Easing.InOutQuad
                    }
                }

                Text {
                    color: "red"
                    font.pixelSize: 12
                    text: model.className
                    x: 5
                    y: model.rect.y > 15 ? -15 : 5

                    Behavior on y {
                        NumberAnimation {
                            duration: 80
                            easing.type: Easing.InOutQuad
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        var settings = client.getSettings()
                        settings["targetId"] = model.objectId
                        client.sendSetSettingsAsk(settings)
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

        StackLayout{
            id: layout
            anchors.fill: parent
            currentIndex: 0

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
                        onClicked: {
                            borderWidthSlider.value = client.horizontalBorder
                            borderHeightSlider.value = client.verticalBorder

                            layout.currentIndex = 1
                        }
                    }

                    Row {
                        RecordButton {
                            recording: client.isRecording
                            onClicked: {
                                var settings = client.getSettings()
                                settings["recording"] = !client.isRecording
                                client.sendSetSettingsAsk(settings)
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
                        onClicked: {
                            var settings = client.getSettings()
                            settings["tracking"] = !client.isTracking
                            client.sendSetSettingsAsk(settings)
                        }
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

            ColumnLayout {
                id: settings
                anchors.fill: parent

                Row {
                    Text { text: qsTr("Border Width:") }
                    Slider {
                        id: borderWidthSlider
                        from: 0.0
                        to: 1.0
                        value: client.horizontalBorder
                    }
                }
                Row {
                    Text { text: qsTr("Border Height:") }
                    Slider {
                        id: borderHeightSlider
                        from: 0.0
                        to: 1.0
                        value: client.verticalBorder
                    }
                }

                Button {
                    Layout.fillWidth: true
                    text: qsTr("Save")
                    onClicked: {

                        var settings = client.getSettings()
                        settings["horizontalBorder"] = borderWidthSlider.value
                        settings["verticalBorder"] = borderHeightSlider.value
                        client.sendSetSettingsAsk(settings)

                        layout.currentIndex = 0
                    }
                }

                Button {
                    Layout.fillWidth: true
                    text: qsTr("Back")
                    onClicked: {
                        layout.currentIndex = 0
                    }
                }
            }
        }
    }
}
