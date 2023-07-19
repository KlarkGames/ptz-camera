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
        }
        Button {
            id: btnConnect
            text: qsTr("Connect")
            onClicked: {
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
