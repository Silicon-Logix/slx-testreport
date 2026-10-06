// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

/// @file
/// @brief Checks import, profile evaluation, and PDF publication together.

#include "../src/reportcontroller.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QtTest>

class WorkflowTests final : public QObject
{
    Q_OBJECT

private slots:
    /// @brief Exercises bundled sample import and checks the resulting PDF structure.
    void exampleToPdf()
    {
        ReportController report;
        QVERIFY(report.loadExample());
        QCOMPARE(report.totalCount(), 15);
        QCOMPARE(report.passedCount(), 10);
        QCOMPARE(report.failedCount(), 2);
        QCOMPARE(report.skippedCount(), 1);
        QCOMPARE(report.invalidCount(), 2);
        QVERIFY(report.profileActive());
        QCOMPARE(report.profileRevision(), QStringLiteral("A"));
        QCOMPARE(report.profileHash().size(), 64);
        QCOMPARE(report.devices().size(), 1);
        QCOMPARE(report.boundedCount(), 9);
        QCOMPARE(report.previewRows().size(), 12);
        QCOMPARE(report.resultsModel()->results().first().measurement, QStringLiteral("12,08"));
        QCOMPARE(report.resultsModel()->data(report.resultsModel()->index(0, 0),
                                             ResultsModel::MeasurementRole).toString(),
                 QStringLiteral("12.08"));
        QCOMPARE(report.previewRows().first().toMap().value(QStringLiteral("value")).toString(),
                 QStringLiteral("12.08"));
        QCOMPARE(report.toleranceHighlights().size(), 6);
        QCOMPARE(report.toleranceHighlights().first().toMap().value(QStringLiteral("name")).toString(),
                 QStringLiteral("Load current"));
        QCOMPARE(report.stationId(), QStringLiteral("BENCH-01"));
        QCOMPARE(report.procedureRevision(), QStringLiteral("A"));
        QVERIFY(report.reportId().startsWith(QStringLiteral("TR-")));
        QCOMPARE(report.sourceHash().size(), 64);
        QVERIFY(report.suggestedPdfUrl().isLocalFile());
        QVERIFY(report.suggestedPdfUrl().fileName().contains(report.serialNumber()));

        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath(QStringLiteral("report.pdf"));
        QVERIFY(report.exportPdf(QUrl::fromLocalFile(path)));
        QFile pdf(path);
        QVERIFY(pdf.open(QIODevice::ReadOnly));
        QVERIFY(pdf.read(5) == QByteArrayLiteral("%PDF-"));
        QVERIFY(pdf.size() > 1000);
        const QString inspectionCopy = qEnvironmentVariable("SLX_REPORT_INSPECTION_COPY");
        if (!inspectionCopy.isEmpty()) {
            pdf.close();
            QVERIFY(QFile::copy(path, inspectionCopy));
        }
    }

    /// @brief Real imports clear sample identity and place the suggested PDF beside the CSV.
    void realImportAfterExample()
    {
        ReportController report;
        QVERIFY(report.loadExample());
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString sourcePath = directory.filePath(QStringLiteral("unit.csv"));
        QFile source(sourcePath);
        QVERIFY(source.open(QIODevice::WriteOnly));
        const QByteArray csv = "matricola;prova;misura;unita;limite_min;limite_max\n"
                               "UNIT-7;Tensione;5.01;V;4.90;5.10\n";
        QCOMPARE(source.write(csv), csv.size());
        source.close();

        QVERIFY(report.importCsv(QUrl::fromLocalFile(sourcePath)));
        QVERIFY(!report.profileActive());
        QCOMPARE(report.serialNumber(), QStringLiteral("UNIT-7"));
        QVERIFY(report.modelName().isEmpty());
        QVERIFY(report.operatorName().isEmpty());
        QVERIFY(report.stationId().isEmpty());
        QVERIFY(report.procedureRevision().isEmpty());
        QCOMPARE(report.totalCount(), 1);
        QCOMPARE(report.sourceHash(),
                 QString::fromLatin1(QCryptographicHash::hash(csv, QCryptographicHash::Sha256).toHex()));
        QCOMPARE(QFileInfo(report.suggestedPdfUrl().toLocalFile()).absolutePath(), directory.path());
        const QString incompletePdf = directory.filePath(QStringLiteral("incomplete.pdf"));
        QVERIFY(!report.exportPdf(QUrl::fromLocalFile(incompletePdf)));
        QVERIFY(!QFile::exists(incompletePdf));
    }

    /// @brief Verifies bounded UI previews and complete multi-page export for a long run.
    void longRunToPdf()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QByteArray csv("serial;test;value;unit;min;max\n");
        for (int i = 0; i < 250; ++i) {
            csv += QStringLiteral("UNIT-250;Channel %1;%2;V;4.90;5.10\n")
                       .arg(i).arg(i == 249 ? QStringLiteral("5.20") : QStringLiteral("5.00"))
                       .toUtf8();
        }
        const QString sourcePath = directory.filePath(QStringLiteral("long-run.csv"));
        QFile source(sourcePath);
        QVERIFY(source.open(QIODevice::WriteOnly));
        QCOMPARE(source.write(csv), csv.size());
        source.close();

        ReportController report;
        QVERIFY(report.importCsv(QUrl::fromLocalFile(sourcePath)));
        QCOMPARE(report.totalCount(), 250);
        QCOMPARE(report.failedCount(), 1);
        QCOMPARE(report.boundedCount(), 250);
        QCOMPARE(report.previewRows().size(), 12);
        QCOMPARE(report.toleranceHighlights().size(), 6);
        report.setModelName(QStringLiteral("SLX-250"));
        report.setOperatorName(QStringLiteral("Test operator"));
        report.setStationId(QStringLiteral("BENCH-1"));
        report.setProcedureRevision(QStringLiteral("B"));
        const QString pdfPath = directory.filePath(QStringLiteral("long-run.pdf"));
        QVERIFY(report.exportPdf(QUrl::fromLocalFile(pdfPath)));
        QFile pdf(pdfPath);
        QVERIFY(pdf.open(QIODevice::ReadOnly));
        QCOMPARE(pdf.read(5), QByteArrayLiteral("%PDF-"));
        QVERIFY(pdf.size() > 10000);
    }

    /// @brief Verifies per-device PDF isolation and unchanged UI selection after batch
    /// export.
    void batchExampleToSeparatePdfs()
    {
        ReportController report;
        QVERIFY(report.loadBatchExample());
        QCOMPARE(report.deviceCount(), 2);
        QCOMPARE(report.batchPassedDevices(), 1);
        QCOMPARE(report.batchAttentionDevices(), 1);
        QCOMPARE(report.serialNumber(), QStringLiteral("BX-1001"));
        QCOMPARE(report.passedCount(), 4);
        QCOMPARE(report.invalidCount(), 0);
        QVERIFY(report.selectDevice(1));
        QCOMPARE(report.serialNumber(), QStringLiteral("BX-1002"));
        QCOMPARE(report.totalCount(), 4);
        QCOMPARE(report.passedCount(), 1);
        QCOMPARE(report.failedCount(), 1);
        QCOMPARE(report.invalidCount(), 2);
        QVERIFY(report.overallOutcome().contains(QStringLiteral("FAIL")));
        QVERIFY(report.resultsModel()->results().last().detail.contains(QStringLiteral("absent from CSV")));

        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QSignalSpy published(&report, &ReportController::batchExportSucceeded);
        QVERIFY(report.exportAllPdfs(QUrl::fromLocalFile(directory.path())));
        QCOMPARE(published.size(), 1);
        const QString outputFolder = published.takeFirst().at(0).toString();
        const QStringList pdfs = QDir(outputFolder).entryList({QStringLiteral("*.pdf")}, QDir::Files);
        QCOMPARE(pdfs.size(), 2);
        QVERIFY(pdfs.at(0).contains(QStringLiteral("BX-1001")));
        QVERIFY(pdfs.at(1).contains(QStringLiteral("BX-1002")));
        QCOMPARE(report.selectedDeviceIndex(), 1);
    }

    /// @brief Verifies reevaluation of existing rows and preservation of the active plan
    /// after a bad replacement.
    void profileAfterCsv()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString sourcePath = directory.filePath(QStringLiteral("raw.csv"));
        QFile source(sourcePath);
        QVERIFY(source.open(QIODevice::WriteOnly));
        source.write("serial;test;value;unit\nUNIT-7;Voltage;5.0;V\n");
        source.close();
        ReportController report;
        QVERIFY(report.importCsv(QUrl::fromLocalFile(sourcePath)));
        QCOMPARE(report.invalidCount(), 1);

        const QString profilePath = directory.filePath(QStringLiteral("plan.json"));
        QFile plan(profilePath);
        QVERIFY(plan.open(QIODevice::WriteOnly));
        plan.write(R"({"schemaVersion":1,"model":"UNIT-7","revision":"C","tests":[
            {"name":"Voltage","kind":"numeric","unit":"V","min":4.9,"max":5.1}]})");
        plan.close();
        QVERIFY(report.loadProfile(QUrl::fromLocalFile(profilePath)));
        QCOMPARE(report.passedCount(), 1);
        QCOMPARE(report.invalidCount(), 0);
        QCOMPARE(report.modelName(), QStringLiteral("UNIT-7"));
        QCOMPARE(report.procedureRevision(), QStringLiteral("C"));

        const QString brokenPath = directory.filePath(QStringLiteral("broken.json"));
        QFile broken(brokenPath);
        QVERIFY(broken.open(QIODevice::WriteOnly));
        broken.write("{bad");
        broken.close();
        QVERIFY(!report.loadProfile(QUrl::fromLocalFile(brokenPath)));
        QCOMPARE(report.profileFile(), QStringLiteral("plan.json"));
        QCOMPARE(report.passedCount(), 1);
        report.clearProfile();
        QVERIFY(!report.profileActive());
        QCOMPARE(report.invalidCount(), 1);
        QVERIFY(report.modelName().isEmpty());
    }
};

QTEST_MAIN(WorkflowTests)
#include "workflow_tests.moc"
