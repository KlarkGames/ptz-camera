import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import QtQuick.Controls.Material

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

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: splashScreenComp
        focus: true
        onFocusChanged: { focus = true; }

        Keys.onBackPressed: {
            stackView.pop()
        }
    }

    Component {
        id: splashScreenComp
        Image {
            fillMode: Image.PreserveAspectFit
            source: "qrc:/res/logo.png"

            Timer {
                running: true
                repeat: false
                interval: 3000
                onTriggered: {
                    stackView.clear(StackView.ReplaceTransition)
                    stackView.push(connectionScreenComp)
                }
            }
        }
    }

    Component {
        id: connectionScreenComp
        ConnectionScreen {
        }
    }
}
