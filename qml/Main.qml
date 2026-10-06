// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

// Composes the import, verification, and report workflow.

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

ApplicationWindow {
    id: window
    visible: true
    width: 1360
    height: 840
    minimumWidth: 1060
    minimumHeight: 680
    title: "SLX Test Report"
    color: appTheme.canvas
    property string notice: ""
    readonly property var appReport: report
    readonly property var appFilteredResults: filteredResultsModel
    readonly property var appCsvComposer: csvComposer

    Theme { id: appTheme }
    AboutDialog {
        id: aboutDialog
        theme: appTheme
        appVersionText: appVersion
        x: (window.width - width) / 2
        y: (window.height - height) / 2
        onLicenseRequested: licenseDialog.open()
        onNoticesRequested: noticesDialog.open()
    }
    LicenseDialog {
        id: licenseDialog
        objectName: "slxLicenseDialog"
        theme: appTheme
        licenseText: embeddedLicenseText
        x: (window.width - width) / 2
        y: (window.height - height) / 2
    }
    LicenseDialog {
        id: noticesDialog
        objectName: "slxNoticesDialog"
        theme: appTheme
        licenseText: embeddedThirdPartyNotices
        documentTitle: "Third-party notices"
        documentSubtitle: "QT  /  MINGW RUNTIME"
        markdown: true
        footerText: "Qt and compiler runtimes retain their own licenses"
        // Open the bundled Qt license here; use the system browser for web links.
        onDocumentLinkActivated: (link) => {
            if (link === "licenses/Qt-Open-Source-LICENSE.txt") qtLicenseDialog.open()
            else if (link.startsWith("https://")) Qt.openUrlExternally(link)
        }
        x: (window.width - width) / 2
        y: (window.height - height) / 2
    }
    LicenseDialog {
        id: qtLicenseDialog
        objectName: "slxQtLicenseDialog"
        theme: appTheme
        licenseText: embeddedQtLicenseText
        documentTitle: "Qt open-source license terms"
        documentSubtitle: "QT  /  THIRD-PARTY TERMS"
        footerText: "Qt terms are separate from the Silicon LogiX license"
        x: (window.width - width) / 2
        y: (window.height - height) / 2
    }
    CsvComposerDialog {
        id: composerDialog
        theme: appTheme
        composer: window.appCsvComposer
        x: (window.width - width) / 2
        y: (window.height - height) / 2
        onOpenRequested: editorOpenDialog.open()
        onSaveRequested: {
            editorSaveDialog.selectedFile = csvComposer.suggestedUrl
            editorSaveDialog.open()
        }
    }

    FileDialog {
        id: importDialog
        title: "Import test results from CSV"
        nameFilters: ["CSV files (*.csv)", "All files (*)"]
        onAccepted: report.importCsv(selectedFile)
    }
    FileDialog {
        id: editorOpenDialog
        title: "Open CSV in editor"
        nameFilters: ["CSV files (*.csv)", "All files (*)"]
        onAccepted: csvComposer.loadCsv(selectedFile)
    }
    FileDialog {
        id: editorSaveDialog
        title: "Save prepared CSV"
        fileMode: FileDialog.SaveFile
        defaultSuffix: "csv"
        nameFilters: ["CSV files (*.csv)"]
        onAccepted: {
            if (csvComposer.saveCsv(selectedFile)) {
                composerDialog.close()
                if (report.importCsv(csvComposer.savedUrl)) {
                    window.notice = "CSV saved and imported: " + csvComposer.savedPath
                    noticeTimer.restart()
                }
            }
        }
    }
    FileDialog {
        id: profileDialog
        title: "Load versioned test profile"
        nameFilters: ["JSON test profiles (*.json)", "All files (*)"]
        onAccepted: report.loadProfile(selectedFile)
    }
    FileDialog {
        id: exportDialog
        title: "Save PDF test report"
        fileMode: FileDialog.SaveFile
        defaultSuffix: "pdf"
        nameFilters: ["PDF document (*.pdf)"]
        onAccepted: report.exportPdf(selectedFile)
    }
    FolderDialog {
        id: batchExportDialog
        title: "Choose folder for batch reports"
        onAccepted: report.exportAllPdfs(selectedFolder)
    }
    Connections {
        target: report
        // Show the destination after the PDF has been saved.
        function onExportSucceeded(path) {
            window.notice = "Report saved: " + path
            noticeTimer.restart()
        }
        // Report the published batch folder and its device count.
        function onBatchExportSucceeded(folder, count) {
            window.notice = count + " reports saved in: " + folder
            noticeTimer.restart()
        }
    }
    Timer {
        id: noticeTimer
        interval: 6000
        onTriggered: window.notice = ""
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        AppHeader {
            Layout.fillWidth: true
            theme: appTheme
            profileActive: report.profileActive
            onExampleRequested: report.loadExample()
            onBatchExampleRequested: report.loadBatchExample()
            onProfileRequested: profileDialog.open()
            onClearProfileRequested: report.clearProfile()
            onImportRequested: importDialog.open()
            onComposeRequested: composerDialog.open()
            onAboutRequested: aboutDialog.open()
        }
        WorkflowStrip {
            Layout.fillWidth: true
            theme: appTheme
            sourceFile: report.sourceFile
            totalCount: report.totalCount
        }
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 28
            Layout.rightMargin: 28
            Layout.topMargin: 19
            Layout.bottomMargin: 19
            spacing: 14

            NotificationBar {
                Layout.fillWidth: true
                error: true
                message: report.errorMessage
                visible: message.length > 0
            }
            NotificationBar {
                Layout.fillWidth: true
                message: window.notice
                visible: message.length > 0
            }
            MetadataPanel {
                Layout.fillWidth: true
                theme: appTheme
                report: window.appReport
            }
            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 16
                ResultsPanel {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumWidth: 585
                    Layout.preferredWidth: 735
                    theme: appTheme
                    report: window.appReport
                    resultsModel: window.appFilteredResults
                }
                ReportPreview {
                    objectName: "slxReportWorkspace"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumWidth: 345
                    Layout.preferredWidth: 445
                    theme: appTheme
                    report: window.appReport
                    onExportRequested: {
                        exportDialog.selectedFile = report.suggestedPdfUrl
                        exportDialog.open()
                    }
                    onExportAllRequested: {
                        batchExportDialog.selectedFolder = report.suggestedBatchFolderUrl
                        batchExportDialog.open()
                    }
                }
            }
        }
    }
    DropArea {
        id: csvDrop
        anchors.fill: parent
        onDropped: function(drop) {
            if (drop.hasUrls && drop.urls.length === 1) {
                report.importCsv(drop.urls[0])
                drop.acceptProposedAction()
            } else {
                window.notice = "Drop one CSV file at a time."
                noticeTimer.restart()
            }
        }
        Rectangle {
            anchors.fill: parent
            visible: csvDrop.containsDrag
            color: "#E9ECFF"
            opacity: 0.96
            border.width: 3
            border.color: appTheme.accent
            Text {
                anchors.centerIn: parent
                text: "Drop CSV to import test results"
                color: appTheme.ink
                font.pixelSize: 22
                font.weight: Font.Bold
            }
        }
    }
}
