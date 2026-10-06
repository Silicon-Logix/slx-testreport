// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

// Defines the shared action control and its interaction states.

import QtQuick
import QtQuick.Controls

Button {
    id: action
    required property var theme
    property bool filled: false
    property bool quiet: false

    implicitHeight: 40
    leftPadding: 18
    rightPadding: 18
    font.pixelSize: 13
    font.weight: Font.DemiBold

    contentItem: Text {
        text: action.text
        color: !action.enabled ? "#9AA3B7" : action.filled ? "white" : action.theme.ink
        font: action.font
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
    background: Rectangle {
        radius: 9
        color: !action.enabled ? "#EBEDF5" : action.filled ? action.theme.accent
               : (action.down ? "#E8EBFA" : action.quiet ? "transparent" : "white")
        border.color: action.filled ? action.theme.accent
                      : action.quiet ? "transparent" : action.theme.lineColor
        border.width: action.quiet ? 0 : 1
        opacity: action.enabled && action.down ? 0.86 : 1
    }
}
