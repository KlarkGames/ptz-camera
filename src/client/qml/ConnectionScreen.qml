import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import QtQuick.Controls.Material
import com.basilevs.multimedia

ColumnLayout {
    ServerListView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.margins: 5

        model: AvailableServerModel {}

        onClicked: (modelIndex) => {
            stackView.push(controlScreenComp, {"ipAddress" : model.data(modelIndex, AvailableServerModel.AddressRole)})
        }
    }

    RowLayout {
        Layout.fillWidth: true
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
