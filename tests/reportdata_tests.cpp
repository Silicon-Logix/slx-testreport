// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

/// @file
/// @brief Checks CSV parsing, source tracing, and result classification.

#include "../src/reportdata.h"

#include <QtTest>

class ReportDataTests final : public QObject
{
    Q_OBJECT

private slots:
    /// @brief Rejects invalid UTF-8 before interpreting CSV columns.
    void rejectsInvalidUtf8()
    {
        QByteArray source("serial;test;value;min;max\nA1;Supply;5.0;4.9;5.1\n");
        source.append(char(0xFF));
        const auto parsed = slx::parseCsv(source);
        QVERIFY(parsed.error.contains(QStringLiteral("UTF-8")));
    }

    /// @brief Separates passing, out-of-limit, and incomplete measurements.
    void classifiesBoundsAndMissingData()
    {
        const QByteArray csv =
            "matricola;prova;misura;unita;limite_min;limite_max\n"
            "A1;Dentro;5;V;5;10\n"
            "A1;Sopra;11;V;5;10\n"
            "A1;Sotto;4;V;5;10\n"
            "A1;Manca;;V;5;10\n"
            "A1;Dimensionless;5;;5;10\n"
            "A1;Senza limiti;5;V;;\n"
            "A1;Limiti invertiti;5;V;10;5\n";
        const auto result = slx::parseCsv(csv);
        QVERIFY(result.error.isEmpty());
        QCOMPARE(result.serial, QStringLiteral("A1"));
        QCOMPARE(result.rows.size(), 7);
        QCOMPARE(result.rows.at(0).outcome, slx::Outcome::Passed);
        QCOMPARE(result.rows.at(1).outcome, slx::Outcome::Failed);
        QCOMPARE(result.rows.at(2).outcome, slx::Outcome::Failed);
        QCOMPARE(result.rows.at(3).outcome, slx::Outcome::Invalid);
        QCOMPARE(result.rows.at(4).outcome, slx::Outcome::Passed);
        QCOMPARE(result.rows.at(5).outcome, slx::Outcome::Invalid);
        QCOMPARE(result.rows.at(6).outcome, slx::Outcome::Invalid);
    }

    /// @brief Preserves quoted CSV fields while parsing a comma decimal value.
    void acceptsQuotedFieldsAndDecimalComma()
    {
        const QByteArray csv =
            "matricola;prova;misura;unita;limite_min;limite_max\n"
            "A1;\"Uscita; \"\"A\"\"\";3,31;V;3,20;3,40\n";
        const auto result = slx::parseCsv(csv);
        QVERIFY(result.error.isEmpty());
        QCOMPARE(result.rows.size(), 1);
        QCOMPARE(result.rows.at(0).testName, QStringLiteral("Uscita; \"A\""));
        QCOMPARE(result.rows.at(0).measurement, QStringLiteral("3,31"));
        QCOMPARE(slx::displayNumber(result.rows.at(0).measurement), QStringLiteral("3.31"));
        QCOMPARE(slx::displayNumber(QStringLiteral("3,3100")), QStringLiteral("3.3100"));
        QCOMPARE(result.rows.at(0).outcome, slx::Outcome::Passed);
    }

    /// @brief Accepts a single bound while rejecting nonfinite readings.
    void acceptsOneSidedLimitsAndRejectsNonFiniteMeasure()
    {
        const QByteArray csv =
            "matricola;prova;misura;unita;limite_min;limite_max\n"
            "A1;Solo minimo;5;V;4;\n"
            "A1;Solo massimo;5;V;;6\n"
            "A1;Non finito;inf;V;0;10\n";
        const auto result = slx::parseCsv(csv);
        QVERIFY(result.error.isEmpty());
        QCOMPARE(result.rows.size(), 3);
        QCOMPARE(result.rows.at(0).outcome, slx::Outcome::Passed);
        QCOMPARE(result.rows.at(1).outcome, slx::Outcome::Passed);
        QCOMPARE(result.rows.at(2).outcome, slx::Outcome::Invalid);
    }

    /// @brief Accepts UTF-8 BOM input and comma-separated headers.
    void acceptsUtf8BomAndCommaDelimiter()
    {
        const QByteArray csv = QByteArray::fromHex("efbbbf") +
            "serial,test,value,unit,min,max\nA1,Voltage,3.3,V,3.2,3.4\n";
        const auto result = slx::parseCsv(csv);
        QVERIFY(result.error.isEmpty());
        QCOMPARE(result.rows.size(), 1);
        QCOMPARE(result.rows.at(0).outcome, slx::Outcome::Passed);
    }

    /// @brief Finds the header delimiter after leading empty lines.
    void detectsSeparatorAfterBlankLines()
    {
        const auto result = slx::parseCsv(
            "\nmatricola;prova;misura;unita;limite_min;limite_max\nA1;Uno;5;V;4;6\n");
        QVERIFY(result.error.isEmpty());
        QCOMPARE(result.rows.size(), 1);
        QCOMPARE(result.rows.at(0).line, 3);
        QCOMPARE(result.rows.at(0).outcome, slx::Outcome::Passed);
    }

    /// @brief Retains each serial and source line when a batch contains malformed records.
    void keepsMixedSerialsForSeparateReportsAndMarksMalformedRows()
    {
        const QByteArray csv =
            "matricola,prova,misura,unita,limite_min,limite_max\n"
            "A1,Uno,3,V,2,4\n"
            "B2,Due,3,V,2,4\n"
            "A1,Tre,3,V,2\n";
        const auto result = slx::parseCsv(csv);
        QVERIFY(result.error.isEmpty());
        QCOMPARE(result.rows.size(), 3);
        QCOMPARE(result.rows.at(0).outcome, slx::Outcome::Passed);
        QCOMPARE(result.rows.at(1).outcome, slx::Outcome::Passed);
        QCOMPARE(result.rows.at(2).outcome, slx::Outcome::Invalid);
        QCOMPARE(result.rows.at(2).line, 4);
    }

    /// @brief Rejects CSV files that cannot supply required report columns.
    void rejectsMissingHeader()
    {
        const auto result = slx::parseCsv("serial;test;unit\nA1;Check;V\n");
        QVERIFY(result.rows.isEmpty());
        QVERIFY(result.error.contains(QStringLiteral("value or status")));
    }

    /// @brief Classifies declared functional outcomes and skipped checks.
    void acceptsFunctionalAndSkippedChecks()
    {
        const auto result = slx::parseCsv(
            "serial;test;status;diagnostic\n"
            "A1;Barcode;PASS;Scanner matched\n"
            "A1;Relay;FAIL;Contact open\n"
            "A1;Programming;NOT_RUN;Image unavailable\n"
            "A1;Communication;ERROR;Timeout\n"
            "A1;Optional check;N/A;Not installed\n"
            "A1;Endurance;ABORTED;Interlock open\n"
            "A1;Unknown;MAYBE;Unclear\n");
        QVERIFY(result.error.isEmpty());
        QCOMPARE(result.rows.size(), 7);
        QCOMPARE(result.rows.at(0).outcome, slx::Outcome::Passed);
        QCOMPARE(result.rows.at(0).detail, QStringLiteral("Scanner matched"));
        QCOMPARE(result.rows.at(1).outcome, slx::Outcome::Failed);
        QCOMPARE(result.rows.at(2).outcome, slx::Outcome::Skipped);
        QCOMPARE(result.rows.at(3).outcome, slx::Outcome::Invalid);
        QCOMPARE(result.rows.at(4).outcome, slx::Outcome::Skipped);
        QCOMPARE(result.rows.at(5).outcome, slx::Outcome::Invalid);
        QCOMPARE(result.rows.at(6).outcome, slx::Outcome::Invalid);
    }

    /// @brief Surfaces conflicts between declared status and measured limits.
    void flagsDeclaredVerdictConflicts()
    {
        const auto result = slx::parseCsv(
            "serial;test;value;min;max;status\n"
            "A1;Above;11;0;10;PASS\n"
            "A1;Inside;5;0;10;FAIL\n"
            "A1;Boundary;10;0;10;PASS\n");
        QVERIFY(result.error.isEmpty());
        QCOMPARE(result.rows.at(0).outcome, slx::Outcome::Invalid);
        QCOMPARE(result.rows.at(1).outcome, slx::Outcome::Invalid);
        QCOMPARE(result.rows.at(2).outcome, slx::Outcome::Passed);
        QVERIFY(result.rows.at(2).hasTwoSidedRange);
        QCOMPARE(result.rows.at(2).rangePosition, 1.0);
    }
};

QTEST_GUILESS_MAIN(ReportDataTests)
#include "reportdata_tests.moc"
