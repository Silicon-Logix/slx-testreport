// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

/// @file
/// @brief Defines source evidence, verdicts, and CSV import results.

#pragma once

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QVector>

namespace slx {

enum class Outcome { Passed, Failed, Skipped, Invalid };
enum class DeclaredStatus { None, Pass, Fail, Skip, Error, Unknown };

/// Keeps imported cells as text and records the physical start line.
/// Derived outcomes and diagnostics never overwrite the source values.
struct TestResult {
    int line = 0;
    QString serial;
    QString testName;
    QString measurement;
    QString unit;
    QString minimum;
    QString maximum;
    QString reportedStatus;
    QString diagnostic;
    Outcome outcome = Outcome::Invalid;
    QString detail;
    bool recordValid = true;
    bool hasTwoSidedRange = false;
    double rangePosition = 0.0;
};

/// A nonempty error means the file cannot be mapped to a report.
/// Individual bad readings remain in rows for operator review.
struct ImportResult {
    QVector<TestResult> rows;
    QString serial;
    QString error;
    QStringList ignoredColumns;
};

/// @brief Parses UTF-8 CSV in source order; structural errors reject the file while bad
/// readings remain Invalid rows.
/// @return ImportResult with error set only when the file cannot be mapped to rows.
ImportResult parseCsv(const QByteArray &contents);

/// @brief Accepts one finite decimal regardless of system locale and writes it to
/// value.
/// @param[out] value Contains a usable finite number only when true is returned.
bool parseNumber(QString text, double &value);

/// @brief Uses a decimal point for valid readings without rounding or changing CSV evidence.
QString displayNumber(QString text);

/// @brief Maps a declared bench status to the supported verdict vocabulary.
DeclaredStatus parseStatus(QString text);

/// @brief Returns the English label displayed for a classified outcome.
QString outcomeText(Outcome outcome);

} // namespace slx
