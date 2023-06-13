// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtMultimedia

import com.basilevs.multimedia

Row {
    id: root
    height: Style.height
    property Camera selected: available ? camera : null
    property bool available: (typeof comboBox.currentValue !== 'undefined')

    ComboBox {
        id: comboBox
        width: Style.widthLong
        height: Style.height
        currentIndex: -1
        model: producer.mountDriver.availablePortNames
        background: StyleRectangle { anchors.fill: parent }
        displayText: typeof currentValue === 'undefined' ? "Unavailable" : currentValue.description
        font.pointSize: Style.fontSize
        textRole: "description"
        onCurrentValueChanged: {
            if (comboBox.currentValue !== '') {
                console.log('Setting port', comboBox.currentValue);
                producer.mountDriver.currentPortName = comboBox.currentValue;
            }
        }
    }
}

