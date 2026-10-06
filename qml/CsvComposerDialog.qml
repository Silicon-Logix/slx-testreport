// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

// Presents the CSV preparation sheet and validation feedback.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: editor
    objectName: "slxCsvComposerDialog"
    required property var theme
    required property var composer
    signal openRequested()
    signal saveRequested()

    readonly property int serialWidth: 105
    readonly property int testWidth: 170
    readonly property int valueWidth: 78
    readonly property int unitWidth: 54
    readonly property int limitWidth: 75
    readonly property int statusWidth: 105
    readonly property int actionWidth: 60
    readonly property int columnSpacing: 6
    readonly property real tableWidth: width - 54
    readonly property real diagnosticWidth: Math.max(155, tableWidth - 770)

    width: Math.min(1180, parent ? parent.width - 40 : 1180)
    height: Math.min(690, parent ? parent.height - 40 : 690)
    modal: true
    focus: true
    padding: 0
    closePolicy: Popup.CloseOnEscape
    background: Rectangle { radius: 14; color: "white"; border.color: editor.theme.lineColor }

    component EditCell: TextField {
        required property int entryIndex
        required property string columnName
        implicitHeight: 33
        color: editor.theme.ink
        font.pixelSize: 11
        selectByMouse: true
        leftPadding: 8
        rightPadding: 7
        onTextEdited: editor.composer.setCell(entryIndex, columnName, text)
        background: Rectangle {
            radius: 5
            color: "white"
            border.color: parent.activeFocus ? editor.theme.accent : editor.theme.lineColor
        }
    }

    Dialog {
        id: resetConfirmation
        title: "Start a new CSV?"
        modal: true
        standardButtons: Dialog.Yes | Dialog.No
        anchors.centerIn: parent
        onAccepted: editor.composer.newDraft()
        contentItem: Text {
            padding: 18
            text: "The current unsaved sheet will be cleared."
            color: editor.theme.ink
            font.pixelSize: 12
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 22
        spacing: 13

        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                spacing: 3
                Text {
                    text: "SLX  /  CSV WORKSPACE"
                    color: editor.theme.accent
                    font.pixelSize: 10
                    font.weight: Font.Bold
                    font.letterSpacing: 1
                }
                Text {
                    text: "Prepare test results"
                    color: editor.theme.ink
                    font.pixelSize: 22
                    font.weight: Font.Bold
                }
            }
            Item { Layout.fillWidth: true }
            Text {
                text: editor.composer.draftName + (editor.composer.dirty ? "  ·  unsaved" : "")
                color: editor.theme.muted
                font.pixelSize: 11
                elide: Text.ElideMiddle
                Layout.maximumWidth: 300
            }
        }

        Text {
            Layout.fillWidth: true
            text: "Enter one test per row. Use the same serial for one device, or add other serials for a batch."
            color: editor.theme.muted
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            ActionButton {
                theme: editor.theme
                text: "New sheet"
                quiet: true
                onClicked: editor.composer.dirty ? resetConfirmation.open() : editor.composer.newDraft()
            }
            ActionButton { theme: editor.theme; text: "Open CSV"; onClicked: editor.openRequested() }
            Item { Layout.fillWidth: true }
            Text {
                text: editor.composer.entryCount + (editor.composer.entryCount === 1 ? " row" : " rows")
                color: editor.theme.muted
                font.pixelSize: 11
            }
            ActionButton { theme: editor.theme; text: "+ Add test"; onClicked: editor.composer.addEntry() }
        }

        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: editor.theme.lineColor }

        Row {
            Layout.fillWidth: true
            Layout.leftMargin: 5
            spacing: editor.columnSpacing
            height: 19
            property color labelColor: editor.theme.muted
            Text { width: editor.serialWidth; text: "SERIAL *"; color: parent.labelColor; font.pixelSize: 10; font.weight: Font.Bold }
            Text { width: editor.testWidth; text: "TEST *"; color: parent.labelColor; font.pixelSize: 10; font.weight: Font.Bold }
            Text { width: editor.valueWidth; text: "VALUE"; color: parent.labelColor; font.pixelSize: 10; font.weight: Font.Bold }
            Text { width: editor.unitWidth; text: "UNIT"; color: parent.labelColor; font.pixelSize: 10; font.weight: Font.Bold }
            Text { width: editor.limitWidth; text: "MIN"; color: parent.labelColor; font.pixelSize: 10; font.weight: Font.Bold }
            Text { width: editor.limitWidth; text: "MAX"; color: parent.labelColor; font.pixelSize: 10; font.weight: Font.Bold }
            Text { width: editor.statusWidth; text: "STATUS"; color: parent.labelColor; font.pixelSize: 10; font.weight: Font.Bold }
            Text { width: editor.diagnosticWidth; text: "DIAGNOSTIC"; color: parent.labelColor; font.pixelSize: 10; font.weight: Font.Bold }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#F8F9FD"
            radius: 7
            border.color: editor.theme.lineColor
            ListView {
                id: entryList
                anchors.fill: parent
                anchors.margins: 5
                clip: true
                spacing: 2
                model: editor.composer
                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                delegate: Rectangle {
                    width: entryList.width
                    height: 42
                    radius: 5
                    color: index % 2 ? "#F1F3FA" : "white"
                    Row {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: editor.columnSpacing
                        EditCell { width: editor.serialWidth; entryIndex: index; columnName: "serial"; text: model.serial; placeholderText: "Unit ID" }
                        EditCell { width: editor.testWidth; entryIndex: index; columnName: "test"; text: model.test; placeholderText: "Test name" }
                        EditCell { width: editor.valueWidth; entryIndex: index; columnName: "value"; text: model.value; placeholderText: "5.00" }
                        EditCell { width: editor.unitWidth; entryIndex: index; columnName: "unit"; text: model.unit; placeholderText: "V" }
                        EditCell { width: editor.limitWidth; entryIndex: index; columnName: "minimum"; text: model.minimum; placeholderText: "Min" }
                        EditCell { width: editor.limitWidth; entryIndex: index; columnName: "maximum"; text: model.maximum; placeholderText: "Max" }
                        EditCell { width: editor.statusWidth; entryIndex: index; columnName: "status"; text: model.status; placeholderText: "PASS / FAIL" }
                        EditCell { width: editor.diagnosticWidth; entryIndex: index; columnName: "diagnostic"; text: model.diagnostic; placeholderText: "Bench note (optional)" }
                        Row {
                            width: editor.actionWidth
                            spacing: 2
                            Button {
                                width: 29
                                height: 33
                                text: "+"
                                font.pixelSize: 15
                                onClicked: editor.composer.duplicateEntry(index)
                                ToolTip.visible: hovered
                                ToolTip.text: "Duplicate row"
                                background: Rectangle { radius: 5; color: parent.down ? "#EDEFFA" : "transparent" }
                            }
                            Button {
                                width: 29
                                height: 33
                                text: "×"
                                font.pixelSize: 17
                                onClicked: editor.composer.removeEntry(index)
                                ToolTip.visible: hovered
                                ToolTip.text: "Remove row"
                                background: Rectangle { radius: 5; color: parent.down ? "#FBEEF0" : "transparent" }
                            }
                        }
                    }
                }
            }
        }

        Text {
            Layout.fillWidth: true
            text: "Numeric: value and limits, or value with an independent test profile. Functional: status PASS or FAIL. SKIPPED and ERROR are also accepted."
            color: editor.theme.muted
            font.pixelSize: 11
            wrapMode: Text.WordWrap
        }
        NotificationBar {
            Layout.fillWidth: true
            error: true
            message: editor.composer.errorMessage
            visible: message.length > 0
        }
        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            ActionButton { theme: editor.theme; text: "Close"; onClicked: editor.close() }
            ActionButton { theme: editor.theme; text: "Save CSV and import"; filled: true; onClicked: editor.saveRequested() }
        }
    }
}
