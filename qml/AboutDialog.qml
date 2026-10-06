// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

// Presents product identity and links to license information.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: about
    objectName: "slxAboutDialog"
    required property var theme
    required property string appVersionText
    signal licenseRequested()
    signal noticesRequested()

    width: 520
    modal: true
    focus: true
    padding: 0
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    background: Rectangle { radius: 16; color: "white"; border.color: about.theme.lineColor }

    contentItem: ColumnLayout {
        spacing: 0
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 154
            color: about.theme.navy
            radius: 16
            Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 16; color: about.theme.navy }
            Rectangle {
                x: 25; y: 17; width: 106; height: 68; radius: 10; color: "#202C59"
                Image { anchors.fill: parent; anchors.margins: 4; source: "qrc:/brand/logo.svg"; fillMode: Image.PreserveAspectFit }
            }
            RowLayout {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.leftMargin: 28
                anchors.rightMargin: 28
                anchors.bottomMargin: 20
                spacing: 12
                Image { Layout.preferredWidth: 40; Layout.preferredHeight: 40; source: "qrc:/brand/report-icon.svg" }
                ColumnLayout {
                    spacing: 1
                    Text { text: "SLX Test Report"; color: "white"; font.pixelSize: 20; font.weight: Font.Bold }
                    Text { text: "Import  /  verify  /  report"; color: "#B7BEE0"; font.pixelSize: 11 }
                }
                Item { Layout.fillWidth: true }
                Text { text: "v" + about.appVersionText; color: "#DCE0FF"; font.pixelSize: 11; font.weight: Font.DemiBold }
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
        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 28
            Layout.rightMargin: 28
            Layout.topMargin: 24
            Layout.bottomMargin: 26
            spacing: 17
            Text {
                Layout.fillWidth: true
                text: "Import bench results, verify versioned test profiles and create traceable PDF reports for each device."
                color: about.theme.ink
                font.pixelSize: 13
                wrapMode: Text.WordWrap
            }
            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: about.theme.lineColor }
            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 18
                rowSpacing: 10
                Text { text: "AUTHOR"; color: about.theme.muted; font.pixelSize: 10; font.weight: Font.Bold; font.letterSpacing: 0.8 }
                Text { text: "Marco Pezzullo · Silicon LogiX"; color: about.theme.ink; font.pixelSize: 12 }
                Text { text: "TECHNOLOGY"; color: about.theme.muted; font.pixelSize: 10; font.weight: Font.Bold; font.letterSpacing: 0.8 }
                Text { text: "C++17 · Qt Quick"; color: about.theme.ink; font.pixelSize: 12 }
                Text { text: "COPYRIGHT"; color: about.theme.muted; font.pixelSize: 10; font.weight: Font.Bold; font.letterSpacing: 0.8 }
                Text { text: "© 2026 Marco Pezzullo (Silicon LogiX)"; color: about.theme.ink; font.pixelSize: 12 }
            }
            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: about.theme.lineColor }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 5
                Text { text: "LICENSE"; color: about.theme.muted; font.pixelSize: 10; font.weight: Font.Bold; font.letterSpacing: 0.8 }
                Text {
                    Layout.fillWidth: true
                    text: "Silicon LogiX Evaluation License 1.0. Study and evaluation are permitted; modifications are for private use only. Commercial use and distribution of modifications require written permission."
                    color: about.theme.ink
                    font.pixelSize: 11
                    wrapMode: Text.WordWrap
                }
                Text {
                    Layout.fillWidth: true
                    text: "Qt and compiler runtimes have separate licenses. View third-party notices"
                    color: about.theme.accent
                    font.pixelSize: 11
                    wrapMode: Text.WordWrap
                    TapHandler { onTapped: { about.close(); about.noticesRequested() } }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                ActionButton { theme: about.theme; text: "Silicon LogiX website"; onClicked: Qt.openUrlExternally("https://www.siliconlogix.it/") }
                ActionButton { theme: about.theme; text: "Read license"; onClicked: { about.close(); about.licenseRequested() } }
                Item { Layout.fillWidth: true }
                ActionButton { theme: about.theme; text: "Close"; filled: true; onClicked: about.close() }
            }
        }
    }
}
