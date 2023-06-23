import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

import com.basilevs.multimedia

Window {
    id: root
    visible: true
    title: "Client"
    width: Style.screenWidth
    height: Style.screenHeigth

    onWidthChanged:{
        Style.calculateRatio(root.width, root.height)
    }

    Client {
        id: client
    }

    Row {
        Button {
            text: "L"
            onClicked: client.sendRotateCmd(Client.DIR_LEFT)
        }
        Button {
            text: "R"
            onClicked: client.sendRotateCmd(Client.DIR_RIGHT)
        }
        Button {
            text: "U"
            onClicked: client.sendRotateCmd(Client.DIR_UP)
        }
        Button {
            text: "D"
            onClicked: client.sendRotateCmd(Client.DIR_DOWN)
        }
    }
}
