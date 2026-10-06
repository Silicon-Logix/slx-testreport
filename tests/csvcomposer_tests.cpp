// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

/// @file
/// @brief Checks CSV editing, round trips, and rejected writes.

#include "../src/csvcomposer.h"
#include "../src/reportdata.h"

#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

class CsvComposerTests final : public QObject
{
    Q_OBJECT

private slots:
    /// @brief CSV save/load preserves edited cells and distinct device serials.
    void savesRoundTripAndBatchRows()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        CsvComposer draft;
        draft.setCell(0, QStringLiteral("serial"), QStringLiteral("BX-1"));
        draft.setCell(0, QStringLiteral("test"), QStringLiteral("Auxiliary; voltage"));
        draft.setCell(0, QStringLiteral("value"), QStringLiteral("5,01"));
        draft.setCell(0, QStringLiteral("unit"), QStringLiteral("V"));
        draft.setCell(0, QStringLiteral("minimum"), QStringLiteral("4,9"));
        draft.setCell(0, QStringLiteral("maximum"), QStringLiteral("5,1"));
        draft.setCell(0, QStringLiteral("diagnostic"), QStringLiteral("Meter said \"stable\"\nafter settling"));
        draft.addEntry();
        draft.setCell(1, QStringLiteral("serial"), QStringLiteral("BX-2"));
        draft.setCell(1, QStringLiteral("test"), QStringLiteral("Barcode scan"));
        draft.setCell(1, QStringLiteral("status"), QStringLiteral("PASS"));
        draft.addEntry(); // A trailing blank sheet row does not enter the CSV.

        const QUrl target = QUrl::fromLocalFile(directory.filePath(QStringLiteral("prepared.csv")));
        QVERIFY2(draft.saveCsv(target), qPrintable(draft.errorMessage()));
        QVERIFY(!draft.dirty());
        QCOMPARE(draft.savedUrl(), target);
        QFile file(target.toLocalFile());
        QVERIFY(file.open(QIODevice::ReadOnly));
        const auto parsed = slx::parseCsv(file.readAll());
        QVERIFY2(parsed.error.isEmpty(), qPrintable(parsed.error));
        QCOMPARE(parsed.rows.size(), 2);
        QCOMPARE(parsed.rows.at(0).testName, QStringLiteral("Auxiliary; voltage"));
        QCOMPARE(parsed.rows.at(0).measurement, QStringLiteral("5,01"));
        QCOMPARE(parsed.rows.at(0).diagnostic, QStringLiteral("Meter said \"stable\"\nafter settling"));
        QCOMPARE(parsed.rows.at(0).outcome, slx::Outcome::Passed);
        QCOMPARE(parsed.rows.at(1).serial, QStringLiteral("BX-2"));
        QCOMPARE(parsed.rows.at(1).outcome, slx::Outcome::Passed);
    }

    /// @brief Invalid drafts leave an existing destination file untouched.
    void invalidDraftCannotReplaceExistingFile()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath(QStringLiteral("prepared.csv"));
        QFile original(path);
        QVERIFY(original.open(QIODevice::WriteOnly));
        QCOMPARE(original.write("original\n"), qint64(9));
        original.close();

        CsvComposer draft;
        draft.setCell(0, QStringLiteral("serial"), QStringLiteral("BX-1"));
        draft.setCell(0, QStringLiteral("test"), QStringLiteral("Supply"));
        draft.setCell(0, QStringLiteral("value"), QStringLiteral("not a number"));
        QVERIFY(!draft.saveCsv(QUrl::fromLocalFile(path)));
        QVERIFY(draft.errorMessage().contains(QStringLiteral("measurement")));
        QVERIFY(original.open(QIODevice::ReadOnly));
        QCOMPARE(original.readAll(), QByteArray("original\n"));
    }

    /// @brief Failed opens retain the unsaved draft.
    void rejectedOpenKeepsUnsavedSheet()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath(QStringLiteral("broken.csv"));
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("wrong;columns\n1;2\n");
        file.close();

        CsvComposer draft;
        draft.setCell(0, QStringLiteral("serial"), QStringLiteral("BX-7"));
        QVERIFY(!draft.loadCsv(QUrl::fromLocalFile(path)));
        QCOMPARE(draft.entryCount(), 1);
        QCOMPARE(draft.data(draft.index(0), CsvComposer::SerialRole).toString(), QStringLiteral("BX-7"));
        QVERIFY(draft.dirty());
    }

    /// @brief Unsupported vendor columns are rejected before the draft is replaced.
    void rejectsColumnsItCannotPreserve()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath(QStringLiteral("extra.csv"));
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("serial;test;value;operator\nBX-1;Supply;5.0;Alice\n");
        file.close();

        CsvComposer draft;
        QVERIFY(!draft.loadCsv(QUrl::fromLocalFile(path)));
        QVERIFY(draft.errorMessage().contains(QStringLiteral("operator")));
        QCOMPARE(draft.entryCount(), 1);
    }
};

QTEST_GUILESS_MAIN(CsvComposerTests)
#include "csvcomposer_tests.moc"
