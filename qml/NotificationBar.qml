// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

// Displays workflow errors and completion messages.

import QtQuick

Rectangle {
    id: bar
    property string message: ""
    property bool error: false

    implicitHeight: 41
    radius: 7
    color: error ? "#FBEEF0" : "#EAF6F2"
    border.color: error ? "#F0D0D5" : "#CDE7DE"
    Text {
        anchors.fill: parent
        anchors.leftMargin: 14
        anchors.rightMargin: 14
        text: bar.message
        color: bar.error ? "#9C3F47" : "#176958"
        font.pixelSize: 12
        verticalAlignment: Text.AlignVCenter
        elide: bar.error ? Text.ElideRight : Text.ElideMiddle
    }
}
