// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

// Displays the embedded license or third-party notice text.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: licenseDialog
    signal documentLinkActivated(string link)
    required property var theme
    required property string licenseText
    property string documentTitle: "Evaluation license"
    property string documentSubtitle: "SILICON LOGIX  /  1.0"
    property bool markdown: false
    property string footerText: "© 2026 Marco Pezzullo · Silicon LogiX"

    width: 650
    height: 590
    modal: true
    focus: true
    padding: 0
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    background: Rectangle { radius: 14; color: "white"; border.color: licenseDialog.theme.lineColor }
    contentItem: ColumnLayout {
        spacing: 0
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 76
            radius: 14
            color: licenseDialog.theme.navy
            Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 14; color: licenseDialog.theme.navy }
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                spacing: 12
                Image { Layout.preferredWidth: 35; Layout.preferredHeight: 35; source: "qrc:/brand/report-icon.svg" }
                ColumnLayout {
                    spacing: 1
                    Text { text: licenseDialog.documentTitle; color: "white"; font.pixelSize: 17; font.weight: Font.Bold }
                    Text { text: licenseDialog.documentSubtitle; color: "#AEB8E1"; font.pixelSize: 10; font.weight: Font.DemiBold; font.letterSpacing: 1 }
                }
            }
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 3
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0; color: "#2948FF" }
                    GradientStop { position: 1; color: "#D100FF" }
                }
            }
        }
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 21
            clip: true
            TextArea {
                text: licenseDialog.licenseText
                textFormat: licenseDialog.markdown ? TextEdit.MarkdownText : TextEdit.PlainText
                // The containing window handles embedded licenses and web links.
                onLinkActivated: (link) => licenseDialog.documentLinkActivated(link)
                readOnly: true
                selectByMouse: true
                wrapMode: TextEdit.Wrap
                font.family: licenseDialog.markdown ? "Segoe UI" : "Consolas"
                font.pixelSize: licenseDialog.markdown ? 12 : 11
                color: licenseDialog.theme.ink
                background: null
            }
        }
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: licenseDialog.theme.lineColor }
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 18
            Text { text: licenseDialog.footerText; color: licenseDialog.theme.muted; font.pixelSize: 10 }
            Item { Layout.fillWidth: true }
            ActionButton { theme: licenseDialog.theme; text: "Close"; filled: true; onClicked: licenseDialog.close() }
        }
    }
}
