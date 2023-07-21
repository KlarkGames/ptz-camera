import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import QtQuick.Controls.Material

ColumnLayout {
    Item { Layout.fillHeight: true }

    RowLayout {
        TextField {
            id: ipAddrTextField
            Layout.margins: 5
            Layout.fillWidth: true
            placeholderText: qsTr("IP address")
            onAccepted: {
                stackView.push(controlScreenComp, {"ipAddress" : ipAddrTextField.text})
            }
        }
        Button {
            id: btnConnect
            text: qsTr("Connect")
            onClicked: {
                ipAddrTextField.accepted()
            }
        }
    }

    Component {
        id: controlScreenComp
        ControlScreen {
        }
    }
}
