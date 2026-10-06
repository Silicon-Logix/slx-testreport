// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

// Provides a labeled metadata editor with change signaling.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: fieldRoot
    required property var theme
    property string caption: ""
    property alias text: input.text
    property string placeholder: ""
    signal edited(string value)
    spacing: 6

    Text {
        text: fieldRoot.caption.toUpperCase()
        color: fieldRoot.theme.muted
        font.pixelSize: 10
        font.weight: Font.Bold
        font.letterSpacing: 0.8
    }
    TextField {
        id: input
        Layout.fillWidth: true
        implicitHeight: 39
        placeholderText: fieldRoot.placeholder
        color: fieldRoot.theme.ink
        font.pixelSize: 13
        selectByMouse: true
        onTextEdited: fieldRoot.edited(text)
        background: Rectangle {
            radius: 7
            color: "white"
            border.color: input.activeFocus ? fieldRoot.theme.accent : fieldRoot.theme.lineColor
            border.width: input.activeFocus ? 2 : 1
        }
    }
}
