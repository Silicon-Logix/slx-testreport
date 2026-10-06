// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

// Presents searchable test results and outcome filters.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: panel
    required property var theme
    required property var report
    required property var resultsModel
    // Header and delegate use the same widths for the first four columns;
    // the outcome column absorbs changes in window width.
    readonly property int testColumnWidth: 168
    readonly property int valueColumnWidth: 82
    readonly property int unitColumnWidth: 48
    readonly property int limitsColumnWidth: 104

    radius: 12
    color: "white"
    border.color: theme.lineColor

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                spacing: 2
                Text { text: "02  /  VERIFY"; color: panel.theme.accent; font.pixelSize: 10; font.weight: Font.Bold; font.letterSpacing: 1 }
                Text { text: "Test results"; color: panel.theme.ink; font.pixelSize: 17; font.weight: Font.Bold }
            }
            Item { Layout.fillWidth: true }
            Rectangle {
                Layout.preferredWidth: 70
                Layout.preferredHeight: 27
                radius: 8
                color: "#EEF0FD"
                Text { anchors.centerIn: parent; text: panel.report.totalCount + " tests"; color: panel.theme.accent; font.pixelSize: 11; font.weight: Font.Bold }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Repeater {
                model: [
                    { label: "Passed", count: panel.report.passedCount, fg: panel.theme.success, bg: "#EAF6F2" },
                    { label: "Failed", count: panel.report.failedCount, fg: panel.theme.failure, bg: "#FBEEF0" },
                    { label: "Skipped", count: panel.report.skippedCount, fg: panel.theme.skipped, bg: "#EEF1F7" },
                    { label: "Invalid", count: panel.report.invalidCount, fg: panel.theme.attention, bg: "#FFF5E7" }
                ]
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: 56
                    radius: 9
                    color: modelData.bg
                    Rectangle { anchors.left: parent.left; anchors.top: parent.top; anchors.bottom: parent.bottom; width: 3; color: modelData.fg }
                    TapHandler { onTapped: statusCombo.currentIndex = index + 1 }
                    Column {
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2
                        Text { text: modelData.count; color: modelData.fg; font.pixelSize: 20; font.weight: Font.Bold }
                        Text { text: modelData.label; color: modelData.fg; font.pixelSize: 10 }
                    }
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            TextField {
                id: searchField
                Layout.fillWidth: true
                implicitHeight: 37
                placeholderText: "Search tests, values, diagnostics"
                color: panel.theme.ink
                font.pixelSize: 12
                onTextChanged: { if (panel.resultsModel) panel.resultsModel.query = text }
                background: Rectangle {
                    radius: 7
                    color: "white"
                    border.color: searchField.activeFocus ? panel.theme.accent : panel.theme.lineColor
                }
            }
            ComboBox {
                id: statusCombo
                Layout.preferredWidth: 150
                implicitHeight: 37
                model: ["All outcomes", "Passed", "Failed", "Skipped", "Invalid"]
                contentItem: Text {
                    leftPadding: 12
                    rightPadding: 25
                    text: statusCombo.displayText
                    color: panel.theme.ink
                    font.pixelSize: 12
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                indicator: Text {
                    x: statusCombo.width - width - 12
                    anchors.verticalCenter: parent.verticalCenter
                    text: "⌄"
                    color: panel.theme.muted
                    font.pixelSize: 16
                }
                background: Rectangle {
                    radius: 7
                    color: "white"
                    border.color: statusCombo.activeFocus ? panel.theme.accent : panel.theme.lineColor
                }
                onCurrentIndexChanged: {
                    if (panel.resultsModel) panel.resultsModel.status = ["all", "pass", "fail", "skipped", "invalid"][currentIndex]
                }
            }
            Text {
                text: resultsList.count + " / " + panel.report.totalCount
                color: panel.theme.muted
                font.pixelSize: 10
            }
        }
        Connections {
            target: panel.report
            // Clear view filters when the controller replaces the selected run.
            function onDataChanged() { searchField.text = ""; statusCombo.currentIndex = 0 }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 36
            color: "#EDEFFA"
            radius: 6
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 11
                anchors.rightMargin: 11
                spacing: 7
                Text { Layout.minimumWidth: panel.testColumnWidth; Layout.maximumWidth: panel.testColumnWidth; text: "TEST"; color: panel.theme.muted; font.pixelSize: 10; font.weight: Font.Bold }
                Text { Layout.minimumWidth: panel.valueColumnWidth; Layout.maximumWidth: panel.valueColumnWidth; text: "VALUE"; color: panel.theme.muted; font.pixelSize: 10; font.weight: Font.Bold }
                Text { Layout.minimumWidth: panel.unitColumnWidth; Layout.maximumWidth: panel.unitColumnWidth; text: "UNIT"; color: panel.theme.muted; font.pixelSize: 10; font.weight: Font.Bold }
                Text { Layout.minimumWidth: panel.limitsColumnWidth; Layout.maximumWidth: panel.limitsColumnWidth; text: "MIN / MAX"; color: panel.theme.muted; font.pixelSize: 10; font.weight: Font.Bold }
                Text { Layout.fillWidth: true; text: "OUTCOME"; color: panel.theme.muted; font.pixelSize: 10; font.weight: Font.Bold }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: resultsList
                anchors.fill: parent
                clip: true
                spacing: 0
                model: panel.resultsModel
                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                delegate: Rectangle {
                    width: ListView.view.width
                    height: 57
                    color: index % 2 ? "#F8F9FD" : "white"
                    HoverHandler { id: rowHover }
                    ToolTip.visible: rowHover.hovered && model.detail.length > 0
                    ToolTip.text: (model.line > 0 ? "CSV line " + model.line : "Test profile") + ": " + model.detail
                    ToolTip.delay: 400
                    Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: panel.theme.lineColor }
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 11
                        anchors.rightMargin: 11
                        spacing: 7
                        ColumnLayout {
                            Layout.minimumWidth: panel.testColumnWidth
                            Layout.maximumWidth: panel.testColumnWidth
                            spacing: 2
                            Text { text: model.testName; color: panel.theme.ink; font.pixelSize: 12; font.weight: Font.DemiBold; elide: Text.ElideRight; Layout.fillWidth: true }
                            Text { text: model.line > 0 ? "CSV line " + model.line : "Required by profile"; color: panel.theme.muted; font.pixelSize: 10; visible: model.detail.length > 0; Layout.fillWidth: true }
                        }
                        Text { Layout.minimumWidth: panel.valueColumnWidth; Layout.maximumWidth: panel.valueColumnWidth; text: model.measurement; color: panel.theme.ink; font.pixelSize: 12; elide: Text.ElideRight }
                        Text { Layout.minimumWidth: panel.unitColumnWidth; Layout.maximumWidth: panel.unitColumnWidth; text: model.unit; color: panel.theme.muted; font.pixelSize: 11; elide: Text.ElideRight }
                        Text { Layout.minimumWidth: panel.limitsColumnWidth; Layout.maximumWidth: panel.limitsColumnWidth; text: model.limits; color: panel.theme.ink; font.pixelSize: 11; elide: Text.ElideRight }
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Text { text: model.outcome; color: panel.theme.statusColor(model.outcomeCode); font.pixelSize: 11; font.weight: Font.Bold; elide: Text.ElideRight; Layout.fillWidth: true }
                            Text { text: model.detail; color: panel.theme.muted; font.pixelSize: 10; elide: Text.ElideRight; Layout.fillWidth: true; visible: text.length > 0 }
                        }
                    }
                }
            }
            Column {
                anchors.centerIn: parent
                spacing: 8
                visible: resultsList.count === 0
                Image { anchors.horizontalCenter: parent.horizontalCenter; width: 48; height: 48; source: "qrc:/brand/report-icon.svg" }
                Text { anchors.horizontalCenter: parent.horizontalCenter; text: panel.report.totalCount === 0 ? "No test data loaded" : "No matching tests"; color: panel.theme.ink; font.pixelSize: 16; font.weight: Font.DemiBold }
                Text { anchors.horizontalCenter: parent.horizontalCenter; text: panel.report.totalCount === 0 ? "Import or drop a bench CSV." : "Change the search or outcome filter."; color: panel.theme.muted; font.pixelSize: 12 }
            }
        }
    }
}
