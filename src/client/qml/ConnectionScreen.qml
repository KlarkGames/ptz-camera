import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import QtQuick.Controls.Material

ColumnLayout {
    property var stack : undefined

    Item { Layout.fillHeight: true }

    RowLayout {
        TextField {
            id: ipAddrTextField
            Layout.margins: 5
            Layout.fillWidth: true
            placeholderText: qsTr("IP address")
        }
        Button {
            id: btnConnect
            text: qsTr("Connect")
            onClicked: {
                console.debug(ipAddrTextField.text)
                stackView.push(controlScreenComp, {"ipAddress" : ipAddrTextField.text})
            }
        }
    }

    Component {
        id: controlScreenComp
        ControlScreen {
        }
    }
}
