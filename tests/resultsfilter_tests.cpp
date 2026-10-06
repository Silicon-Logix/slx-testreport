// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

/// @file
/// @brief Checks view filtering without mutating source results.

#include "../src/resultsfiltermodel.h"
#include "../src/resultsmodel.h"

#include <QtTest>

class ResultsFilterTests final : public QObject
{
    Q_OBJECT

private slots:
    /// @brief Outcome and text filters combine while the source rows stay intact.
    void statusAndText()
    {
        ResultsModel source;
        QVector<slx::TestResult> rows;
        slx::TestResult passed;
        passed.testName = QStringLiteral("Alimentazione");
        passed.measurement = QStringLiteral("12.08");
        passed.outcome = slx::Outcome::Passed;
        rows.append(passed);
        slx::TestResult failed;
        failed.testName = QStringLiteral("Corrente");
        failed.measurement = QStringLiteral("0.42");
        failed.outcome = slx::Outcome::Failed;
        rows.append(failed);
        slx::TestResult skipped;
        skipped.testName = QStringLiteral("Programming");
        skipped.outcome = slx::Outcome::Skipped;
        rows.append(skipped);
        slx::TestResult invalid;
        invalid.testName = QStringLiteral("Temperatura");
        invalid.detail = QStringLiteral("Misura assente");
        rows.append(invalid);
        source.setResults(rows);

        ResultsFilterModel filtered;
        filtered.setSourceModel(&source);
        QCOMPARE(filtered.rowCount(), 4);
        filtered.setStatus(QStringLiteral("fail"));
        QCOMPARE(filtered.rowCount(), 1);
        filtered.setQuery(QStringLiteral("CORRENTE"));
        QCOMPARE(filtered.rowCount(), 1);
        filtered.setQuery(QStringLiteral("12.08"));
        QCOMPARE(filtered.rowCount(), 0);
        filtered.setStatus(QStringLiteral("all"));
        QCOMPARE(filtered.rowCount(), 1);
        filtered.setQuery({});
        filtered.setStatus(QStringLiteral("skipped"));
        QCOMPARE(filtered.rowCount(), 1);
        filtered.setStatus(QStringLiteral("all"));
        QCOMPARE(filtered.rowCount(), 4);
        QCOMPARE(source.rowCount(), 4);
    }
};

QTEST_GUILESS_MAIN(ResultsFilterTests)
#include "resultsfilter_tests.moc"
