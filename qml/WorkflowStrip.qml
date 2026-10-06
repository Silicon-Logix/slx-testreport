// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

// Shows the three visible stages of the test-report workflow.

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: strip
    required property var theme
    property string sourceFile: ""
    property int totalCount: 0

    implicitHeight: 82
    color: theme.navy
    Rectangle {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        width: parent.width
        height: 2
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0; color: "#2948FF" }
            GradientStop { position: 1; color: "#D100FF" }
        }
    }
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 30
        anchors.rightMargin: 30
        spacing: 17
        Repeater {
            model: [
                { number: "01", title: "IMPORT", detail: "Bench CSV" },
                { number: "02", title: "VERIFY", detail: "Limits and verdicts" },
                { number: "03", title: "REPORT", detail: "Analysis and PDF" }
            ]
            RowLayout {
                spacing: 10
                Rectangle {
                    Layout.preferredWidth: 29
                    Layout.preferredHeight: 29
                    radius: 8
                    color: strip.totalCount > 0 ? "#333F79" : "#252D55"
                    border.color: strip.totalCount > 0 ? "#626DEB" : "#465074"
                    Text { anchors.centerIn: parent; text: modelData.number; color: "#D7DCFF"; font.pixelSize: 10; font.weight: Font.Bold }
                }
                ColumnLayout {
                    spacing: 1
                    Text { text: modelData.title; color: "white"; font.pixelSize: 11; font.weight: Font.Bold; font.letterSpacing: 1.2 }
                    Text { text: modelData.detail; color: "#A7AFD1"; font.pixelSize: 10 }
                }
                Text {
                    visible: index < 2
                    Layout.leftMargin: 8
                    text: "›"
                    color: "#7C86B3"
                    font.pixelSize: 23
                }
            }
        }
        Item { Layout.fillWidth: true }
        ColumnLayout {
            Layout.maximumWidth: 285
            spacing: 3
            Text { text: strip.totalCount > 0 ? "SESSION LOADED" : "WAITING FOR DATA"; color: "#A7AFD1"; font.pixelSize: 9; font.weight: Font.Bold; font.letterSpacing: 1 }
            Text {
                Layout.fillWidth: true
                text: strip.sourceFile.length > 0 ? strip.sourceFile : "Import or drop a CSV"
                color: "white"
                font.pixelSize: 11
                elide: Text.ElideMiddle
                horizontalAlignment: Text.AlignRight
            }
        }
    }
}
