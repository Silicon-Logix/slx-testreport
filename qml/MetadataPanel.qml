// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

// Collects report identity fields required for export.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: panel
    required property var theme
    required property var report

    implicitHeight: 139
    radius: 12
    color: "white"
    border.color: theme.lineColor

    Rectangle { x: 0; y: 17; width: 3; height: 34; color: panel.theme.accent }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 17
        spacing: 13
        RowLayout {
            Layout.fillWidth: true
            Text { text: "01  /  DEVICE IDENTITY"; color: panel.theme.accent; font.pixelSize: 10; font.weight: Font.Bold; font.letterSpacing: 1 }
            Text {
                text: panel.report.profileActive
                      ? "PROFILE  " + panel.report.profileFile + "  ·  REV " + panel.report.profileRevision
                      : "CSV LIMITS ONLY"
                color: panel.report.profileActive ? panel.theme.success : panel.theme.attention
                font.pixelSize: 10
                font.weight: Font.DemiBold
                elide: Text.ElideMiddle
                Layout.minimumWidth: 90
                Layout.preferredWidth: 250
                Layout.maximumWidth: 340
                HoverHandler { id: profileHover }
                ToolTip.visible: profileHover.hovered && panel.report.profileActive
                ToolTip.text: "SHA-256  " + panel.report.profileHash
            }
            Item { Layout.fillWidth: true }
            Text {
                visible: panel.report.deviceCount > 1
                text: (panel.report.selectedDeviceIndex + 1) + " / " + panel.report.deviceCount + " DEVICES"
                color: panel.theme.muted
                font.pixelSize: 10
                font.weight: Font.Bold
            }
            ComboBox {
                id: devicePicker
                visible: panel.report.deviceCount > 1
                Layout.preferredWidth: 230
                implicitHeight: 32
                model: panel.report.devices
                textRole: "label"
                currentIndex: panel.report.selectedDeviceIndex
                onActivated: function(index) { panel.report.selectDevice(index) }
                contentItem: Text {
                    leftPadding: 10
                    rightPadding: 23
                    text: devicePicker.displayText
                    color: panel.theme.ink
                    font.pixelSize: 11
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                indicator: Text {
                    x: devicePicker.width - width - 10
                    anchors.verticalCenter: parent.verticalCenter
                    text: "⌄"
                    color: panel.theme.muted
                    font.pixelSize: 16
                }
                background: Rectangle {
                    radius: 7
                    color: "#F3F5FC"
                    border.color: panel.theme.lineColor
                }
            }
            Text {
                text: panel.report.reportId.length > 0 ? panel.report.reportId : "Report details"
                color: panel.theme.muted
                font.pixelSize: 10
                elide: Text.ElideMiddle
                Layout.preferredWidth: 150
                Layout.maximumWidth: 180
            }
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: 14
            MetaField {
                theme: panel.theme
                Layout.fillWidth: true
                Layout.preferredWidth: 190
                caption: "Model"
                placeholder: "e.g. SLX-CTRL-01"
                text: panel.report.modelName
                enabled: !panel.report.profileActive
                onEdited: function(value) { panel.report.modelName = value }
            }
            MetaField {
                theme: panel.theme
                Layout.fillWidth: true
                Layout.preferredWidth: 155
                caption: "Serial number"
                text: panel.report.serialNumber.length > 0 ? panel.report.serialNumber : "From CSV"
                enabled: false
            }
            MetaField {
                theme: panel.theme
                Layout.fillWidth: true
                Layout.preferredWidth: 155
                caption: "Test date · YYYY-MM-DD"
                text: panel.report.testDate
                onEdited: function(value) { panel.report.testDate = value }
            }
            MetaField {
                theme: panel.theme
                Layout.fillWidth: true
                Layout.preferredWidth: 175
                caption: "Operator"
                placeholder: "Operator name"
                text: panel.report.operatorName
                onEdited: function(value) { panel.report.operatorName = value }
            }
            MetaField {
                theme: panel.theme
                Layout.fillWidth: true
                Layout.preferredWidth: 160
                caption: "Station"
                placeholder: "Bench ID"
                text: panel.report.stationId
                onEdited: function(value) { panel.report.stationId = value }
            }
            MetaField {
                theme: panel.theme
                Layout.fillWidth: true
                Layout.preferredWidth: 150
                caption: "Procedure rev."
                placeholder: "e.g. A"
                text: panel.report.procedureRevision
                enabled: !panel.report.profileActive
                onEdited: function(value) { panel.report.procedureRevision = value }
            }
        }
    }
}
