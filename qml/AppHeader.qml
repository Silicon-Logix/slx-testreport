// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

// Provides branded navigation and top-level actions.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: header
    required property var theme
    property bool profileActive: false
    signal exampleRequested()
    signal batchExampleRequested()
    signal profileRequested()
    signal clearProfileRequested()
    signal importRequested()
    signal composeRequested()
    signal aboutRequested()

    implicitHeight: 82
    color: "white"
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 4
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0; color: "#2948FF" }
            GradientStop { position: 1; color: "#D100FF" }
        }
    }
    Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: header.theme.lineColor }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 28
        anchors.rightMargin: 28
        anchors.topMargin: 5
        spacing: 15
        Rectangle {
            Layout.preferredWidth: 96
            Layout.preferredHeight: 62
            radius: 7
            color: header.theme.navy
            Image {
                anchors.fill: parent
                anchors.margins: 3
                source: "qrc:/brand/logo.svg"
                fillMode: Image.PreserveAspectFit
                smooth: true
            }
        }
        Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 36; color: header.theme.lineColor }
        Image {
            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            source: "qrc:/brand/report-icon.svg"
            fillMode: Image.PreserveAspectFit
            smooth: true
        }
        ColumnLayout {
            spacing: 1
            Text { text: "Test Report"; color: header.theme.ink; font.pixelSize: 19; font.weight: Font.Bold }
            Text { text: "TEST EVIDENCE / TRACEABILITY"; color: header.theme.muted; font.pixelSize: 9; font.weight: Font.DemiBold; font.letterSpacing: 1.1 }
        }
        Item { Layout.fillWidth: true }
        ActionButton {
            theme: header.theme
            text: "Options"
            quiet: true
            onClicked: optionsMenu.open()
        }
        ActionButton {
            theme: header.theme
            text: "Create CSV"
            onClicked: header.composeRequested()
        }
        ActionButton {
            theme: header.theme
            text: "Import CSV"
            filled: true
            onClicked: header.importRequested()
        }
    }
    Menu {
        id: optionsMenu
        x: header.width - width - 158
        y: header.height - 2
        MenuItem { text: "Load single-device example"; onTriggered: header.exampleRequested() }
        MenuItem { text: "Load batch example"; onTriggered: header.batchExampleRequested() }
        MenuSeparator {}
        MenuItem { text: "Create CSV"; onTriggered: header.composeRequested() }
        MenuSeparator {}
        MenuItem { text: "Load test profile"; onTriggered: header.profileRequested() }
        MenuItem { text: "Clear test profile"; enabled: header.profileActive; onTriggered: header.clearProfileRequested() }
        MenuSeparator {}
        MenuItem { text: "About"; onTriggered: header.aboutRequested() }
    }
}
