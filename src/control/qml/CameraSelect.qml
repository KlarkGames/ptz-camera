// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtMultimedia

Row {
    id: root
    height: Style.height
    property bool available: (typeof comboBox.currentValue !== 'undefined') && cameraSwitch.checked


    MediaDevices { id: mediaDevices }

    Switch {
        id: cameraSwitch
        anchors.verticalCenter: parent.verticalCenter
        checked: false
    }

    ComboBox {
        id: comboBox
        width: Style.widthLong
        height: Style.height
        background: StyleRectangle { anchors.fill: parent }
        model: mediaDevices.videoInputs
        displayText: typeof currentValue === 'undefined' ? "Unavailable" : currentValue.description
        font.pointSize: Style.fontSize
        textRole: "description"

        onActivated: {
            if (typeof comboBox.currentValue !== 'undefined') {
                console.log('Setting camera', comboBox.currentValue);
                cameraWrapper.camera.cameraDevice = currentValue;
            }
        }
    }
}
