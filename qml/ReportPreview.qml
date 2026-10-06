// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

// Selected-device analysis and summary preview. PDF export includes every result.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: panel
    required property var theme
    required property var report
    property int currentTab: 0
    signal exportRequested()
    signal exportAllRequested()

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
                Text { text: "03  /  REPORT"; color: panel.theme.accent; font.pixelSize: 10; font.weight: Font.Bold; font.letterSpacing: 1 }
                Text { text: "Report workspace"; color: panel.theme.ink; font.pixelSize: 17; font.weight: Font.Bold }
            }
            Item { Layout.fillWidth: true }
            Text { text: "A4  /  PDF"; color: panel.theme.muted; font.pixelSize: 11 }
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: 6
            Repeater {
                model: ["Analysis", "PDF preview"]
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: 32
                    radius: 7
                    color: panel.currentTab === index ? panel.theme.navy : "#F3F5FB"
                    border.color: panel.currentTab === index ? panel.theme.navy : panel.theme.lineColor
                    Text {
                        anchors.centerIn: parent
                        text: modelData
                        color: panel.currentTab === index ? "white" : panel.theme.muted
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                    }
                    TapHandler { onTapped: panel.currentTab = index }
                }
            }
        }
        AnalysisView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: panel.currentTab === 0
            theme: panel.theme
            report: panel.report
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: panel.currentTab === 1
            radius: 7
            color: "#EDF0F8"
            border.color: panel.theme.lineColor
            Flickable {
                id: previewFlick
                anchors.fill: parent
                anchors.margins: 12
                clip: true
                contentWidth: width
                contentHeight: previewPage.implicitHeight + 24
                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                Rectangle {
                    id: previewPage
                    x: 8
                    y: 8
                    width: previewFlick.width - 16
                    implicitHeight: previewContent.implicitHeight + 40
                    color: "white"
                    border.color: panel.theme.lineColor
                    Rectangle {
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
                        id: previewContent
                        x: 20
                        y: 20
                        width: previewPage.width - 40
                        spacing: 12
                        Rectangle {
                            width: parent.width
                            height: 38
                            radius: 4
                            color: panel.theme.navy
                            Image {
                                x: 5
                                y: 2
                                width: 65
                                height: 34
                                source: "qrc:/brand/logo.svg"
                                fillMode: Image.PreserveAspectFit
                            }
                            Text {
                                anchors.right: parent.right
                                anchors.rightMargin: 9
                                anchors.verticalCenter: parent.verticalCenter
                                text: "TEST REPORT"
                                color: "white"
                                font.pixelSize: 9
                                font.weight: Font.Bold
                                font.letterSpacing: 0.7
                            }
                        }
                        Rectangle { width: parent.width; height: 1; color: panel.theme.lineColor }
                        Text { width: parent.width; text: "Device test report"; color: panel.theme.ink; font.pixelSize: 20; font.weight: Font.Bold; elide: Text.ElideRight }
                        Text { text: "Measurements, limits and declared outcomes"; color: panel.theme.muted; font.pixelSize: 10 }
                        Rectangle {
                            width: parent.width
                            height: 111
                            radius: 5
                            color: "#F3F5FC"
                            GridLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                columns: 2
                                columnSpacing: 10
                                rowSpacing: 3
                                Text { text: "MODEL  " + (panel.report.modelName || "—"); color: panel.theme.ink; font.pixelSize: 10; elide: Text.ElideRight; Layout.fillWidth: true }
                                Text { text: "SERIAL  " + (panel.report.serialNumber || "—"); color: panel.theme.ink; font.pixelSize: 10; elide: Text.ElideRight; Layout.fillWidth: true }
                                Text { text: "DATE  " + (panel.report.testDate || "—"); color: panel.theme.ink; font.pixelSize: 10; elide: Text.ElideRight; Layout.fillWidth: true }
                                Text { text: "OPERATOR  " + (panel.report.operatorName || "—"); color: panel.theme.ink; font.pixelSize: 10; elide: Text.ElideRight; Layout.fillWidth: true }
                                Text { text: "STATION  " + (panel.report.stationId || "—"); color: panel.theme.ink; font.pixelSize: 10; elide: Text.ElideRight; Layout.fillWidth: true }
                                Text { text: "PROCEDURE  " + (panel.report.procedureRevision || "—"); color: panel.theme.ink; font.pixelSize: 10; elide: Text.ElideRight; Layout.fillWidth: true }
                            }
                        }
                        Column {
                            spacing: 3
                            Text { text: "OVERALL OUTCOME"; color: panel.theme.muted; font.pixelSize: 9; font.weight: Font.Bold }
                            Text {
                                width: previewContent.width
                                text: panel.report.overallOutcome
                                color: panel.report.failedCount > 0 ? panel.theme.failure
                                     : panel.report.invalidCount > 0 ? panel.theme.attention
                                     : panel.report.skippedCount > 0 ? panel.theme.skipped : panel.theme.success
                                font.pixelSize: 14
                                font.weight: Font.Bold
                                elide: Text.ElideRight
                            }
                            Text {
                                width: previewContent.width
                                text: panel.report.totalCount + " tests · " + panel.report.passedCount + " passed · "
                                      + panel.report.failedCount + " failed · " + panel.report.skippedCount + " skipped · "
                                      + panel.report.invalidCount + " invalid"
                                color: panel.theme.muted
                                font.pixelSize: 9
                                elide: Text.ElideRight
                            }
                        }
                        Column {
                            width: parent.width
                            spacing: 3
                            Text { text: "TRACEABILITY"; color: panel.theme.muted; font.pixelSize: 9; font.weight: Font.Bold }
                            Text { width: parent.width; text: "ID  " + (panel.report.reportId || "—"); color: panel.theme.ink; font.pixelSize: 9; elide: Text.ElideRight }
                            Text { width: parent.width; text: "CSV  " + (panel.report.sourceFile || "—") + "  ·  " + (panel.report.importedAt || "—"); color: panel.theme.muted; font.pixelSize: 9; elide: Text.ElideRight }
                            Text {
                                width: parent.width
                                text: "SHA-256  " + (panel.report.sourceHash || "—")
                                color: panel.theme.muted
                                font.pixelSize: 9
                                elide: Text.ElideMiddle
                                HoverHandler { id: hashHover }
                                ToolTip.visible: hashHover.hovered && panel.report.sourceHash.length > 0
                                ToolTip.text: panel.report.sourceHash
                            }
                            Text {
                                visible: panel.report.profileActive
                                width: parent.width
                                text: "PROFILE  " + panel.report.profileFile + "  ·  REV " + panel.report.profileRevision
                                color: panel.theme.ink
                                font.pixelSize: 9
                                elide: Text.ElideRight
                            }
                            Text {
                                visible: panel.report.profileActive
                                width: parent.width
                                text: "PROFILE SHA-256  " + panel.report.profileHash
                                color: panel.theme.muted
                                font.pixelSize: 9
                                elide: Text.ElideMiddle
                            }
                        }
                        Rectangle { width: parent.width; height: 1; color: panel.theme.lineColor }
                        Repeater {
                            model: panel.report.previewRows
                            Rectangle {
                                width: previewContent.width
                                height: 39
                                color: "transparent"
                                RowLayout {
                                    anchors.fill: parent
                                    spacing: 5
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 1
                                        Text { Layout.fillWidth: true; text: modelData.name; color: panel.theme.ink; font.pixelSize: 10; font.weight: Font.DemiBold; elide: Text.ElideRight }
                                        Text { Layout.fillWidth: true; text: modelData.value + " " + modelData.unit + "   ·   " + modelData.limits; color: panel.theme.muted; font.pixelSize: 9; elide: Text.ElideRight }
                                    }
                                    Text { text: modelData.outcome; color: panel.theme.statusColor(modelData.code); font.pixelSize: 9; font.weight: Font.Bold }
                                }
                                Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: panel.theme.lineColor }
                            }
                        }
                        Text {
                            visible: panel.report.totalCount > panel.report.previewRows.length
                            text: "+ " + (panel.report.totalCount - panel.report.previewRows.length) + " more tests in the complete PDF"
                            color: panel.theme.accent
                            font.pixelSize: 9
                            font.weight: Font.DemiBold
                        }
                        Text { text: "Generated with SLX Test Report"; color: panel.theme.muted; font.pixelSize: 9 }
                    }
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: 7
            ActionButton {
                Layout.fillWidth: true
                theme: panel.theme
                text: panel.report.deviceCount > 1 ? "Export selected" : "Export PDF report"
                filled: true
                enabled: panel.report.totalCount > 0
                onClicked: panel.exportRequested()
            }
            ActionButton {
                visible: panel.report.deviceCount > 1
                Layout.fillWidth: true
                theme: panel.theme
                text: "Export all PDFs"
                enabled: panel.report.deviceCount > 1
                onClicked: panel.exportAllRequested()
            }
        }
    }
}
