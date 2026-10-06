// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

// Shows outcome distribution and readings near their limits.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: analysis
    required property var theme
    required property var report
    radius: 7
    color: "#F8F9FE"
    border.color: theme.lineColor

    ScrollView {
        anchors.fill: parent
        anchors.margins: 1
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        Column {
            width: analysis.width - 22
            anchors.horizontalCenter: parent.horizontalCenter
            topPadding: 17
            bottomPadding: 18
            spacing: 14

            Rectangle {
                width: parent.width
                height: 100
                radius: 9
                color: analysis.theme.navy
                Rectangle {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    width: parent.width
                    height: 3
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0; color: "#2948FF" }
                        GradientStop { position: 1; color: "#D100FF" }
                    }
                }
                Column {
                    anchors.fill: parent
                    anchors.margins: 13
                    spacing: 4
                    Text { text: "RUN VERDICT"; color: "#ACB6DC"; font.pixelSize: 9; font.weight: Font.Bold; font.letterSpacing: 1 }
                    Text {
                        width: parent.width
                        text: analysis.report.overallOutcome
                        color: "white"
                        font.pixelSize: 18
                        font.weight: Font.Bold
                        elide: Text.ElideRight
                    }
                    Text {
                        text: analysis.report.totalCount + " tests · " + (analysis.report.serialNumber || "no serial loaded")
                        color: "#C8D0EE"
                        font.pixelSize: 11
                    }
                    Text {
                        visible: analysis.report.deviceCount > 1
                        text: "BATCH  " + analysis.report.batchPassedDevices + " passed · "
                              + analysis.report.batchAttentionDevices + " need attention"
                        color: analysis.report.batchAttentionDevices > 0 ? "#FFD0D1" : "#BCEAD9"
                        font.pixelSize: 10
                        font.weight: Font.DemiBold
                    }
                }
            }

            Column {
                width: parent.width
                spacing: 9
                Text { text: "OUTCOME DISTRIBUTION"; color: analysis.theme.ink; font.pixelSize: 11; font.weight: Font.Bold; font.letterSpacing: 0.7 }
                Text { text: "All evaluated tests, including profile-required checks"; color: analysis.theme.muted; font.pixelSize: 10 }
                Rectangle {
                    id: outcomeRail
                    width: parent.width
                    height: 18
                    radius: 6
                    color: "#E8EBF5"
                    clip: true
                    Row {
                        anchors.fill: parent
                        Rectangle { width: analysis.report.totalCount ? outcomeRail.width * analysis.report.passedCount / analysis.report.totalCount : 0; height: parent.height; color: analysis.theme.success }
                        Rectangle { width: analysis.report.totalCount ? outcomeRail.width * analysis.report.failedCount / analysis.report.totalCount : 0; height: parent.height; color: analysis.theme.failure }
                        Rectangle { width: analysis.report.totalCount ? outcomeRail.width * analysis.report.skippedCount / analysis.report.totalCount : 0; height: parent.height; color: analysis.theme.skipped }
                        Rectangle { width: analysis.report.totalCount ? outcomeRail.width * analysis.report.invalidCount / analysis.report.totalCount : 0; height: parent.height; color: analysis.theme.attention }
                    }
                }
                RowLayout {
                    width: parent.width
                    spacing: 7
                    Repeater {
                        model: [
                            { name: "Pass", count: analysis.report.passedCount, color: analysis.theme.success },
                            { name: "Fail", count: analysis.report.failedCount, color: analysis.theme.failure },
                            { name: "Skipped", count: analysis.report.skippedCount, color: analysis.theme.skipped },
                            { name: "Invalid", count: analysis.report.invalidCount, color: analysis.theme.attention }
                        ]
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            Rectangle { Layout.preferredWidth: 6; Layout.preferredHeight: 6; radius: 3; color: modelData.color }
                            Text { text: modelData.count + " " + modelData.name; color: analysis.theme.muted; font.pixelSize: 9 }
                        }
                    }
                }
            }

            Rectangle { width: parent.width; height: 1; color: analysis.theme.lineColor }

            Column {
                width: parent.width
                spacing: 10
                Text { text: "LIMIT PROXIMITY"; color: analysis.theme.ink; font.pixelSize: 11; font.weight: Font.Bold; font.letterSpacing: 0.7 }
                Text {
                    width: parent.width
                    text: analysis.report.boundedCount + " numeric tests with two limits · 6 closest or outside shown"
                    color: analysis.theme.muted
                    font.pixelSize: 10
                    wrapMode: Text.WordWrap
                }
                Text {
                    visible: analysis.report.boundedCount === 0
                    width: parent.width
                    text: analysis.report.totalCount === 0 ? "Import a CSV to inspect limit margins." : "No two-sided numeric limits in this run."
                    color: analysis.theme.muted
                    font.pixelSize: 11
                    wrapMode: Text.WordWrap
                }
                Repeater {
                    model: analysis.report.toleranceHighlights
                    Rectangle {
                        width: parent.width
                        height: 76
                        radius: 7
                        color: "white"
                        border.color: analysis.theme.lineColor
                        Column {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 7
                            RowLayout {
                                width: parent.width
                                spacing: 5
                                Text {
                                    Layout.fillWidth: true
                                    text: modelData.name
                                    color: analysis.theme.ink
                                    font.pixelSize: 11
                                    font.weight: Font.DemiBold
                                    elide: Text.ElideRight
                                }
                                Text {
                                    text: modelData.value
                                    color: modelData.failed ? analysis.theme.failure : analysis.theme.accent
                                    font.pixelSize: 11
                                    font.weight: Font.Bold
                                }
                            }
                            Rectangle {
                                id: limitRail
                                width: parent.width
                                height: 9
                                radius: 4
                                color: "#E9EDF9"
                                Rectangle {
                                    anchors.fill: parent
                                    radius: parent.radius
                                    color: modelData.failed ? "#F7DDE0" : "#D7EBE4"
                                }
                                Rectangle {
                                    x: Math.max(0, Math.min(1, modelData.position)) * (parent.width - width)
                                    y: -3
                                    width: 7
                                    height: 15
                                    radius: 3
                                    color: modelData.failed ? analysis.theme.failure : analysis.theme.accent
                                }
                            }
                            RowLayout {
                                width: parent.width
                                Text { text: "MIN " + modelData.minimum; color: analysis.theme.muted; font.pixelSize: 9 }
                                Item { Layout.fillWidth: true }
                                Text {
                                    visible: modelData.failed
                                    text: modelData.position < 0 ? "BELOW LIMIT" : "ABOVE LIMIT"
                                    color: analysis.theme.failure
                                    font.pixelSize: 9
                                    font.weight: Font.Bold
                                }
                                Item { Layout.fillWidth: true }
                                Text { text: "MAX " + modelData.maximum; color: analysis.theme.muted; font.pixelSize: 9 }
                            }
                        }
                    }
                }
                Text {
                    visible: analysis.report.boundedCount > 0
                    width: parent.width
                    text: "Each rail is scaled to its own limits. Functional, one-sided and invalid tests remain in the results table and PDF."
                    color: analysis.theme.muted
                    font.pixelSize: 9
                    wrapMode: Text.WordWrap
                }
            }
        }
    }
}
