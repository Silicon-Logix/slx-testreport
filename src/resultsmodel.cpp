// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

/// @file
/// @brief Adapts evaluated test rows to stable QML roles.

#include "resultsmodel.h"

#include <utility>

/// @brief Creates the read-only Qt model used by result delegates.
ResultsModel::ResultsModel(QObject *parent) : QAbstractListModel(parent) {}

/// @brief Reports only top-level result rows because the model has no children.
int ResultsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_rows.size();
}

/// @brief Maps a result into display roles, preserving diagnostics and source line
/// separately.
QVariant ResultsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size()) {
        return {};
    }
    const auto &row = m_rows.at(index.row());
    switch (role) {
    case TestNameRole: return row.testName.isEmpty() ? QStringLiteral("Unnamed test") : row.testName;
    case MeasurementRole: return row.measurement.isEmpty() ? QStringLiteral("—")
                                                           : slx::displayNumber(row.measurement);
    case UnitRole: return row.unit.isEmpty() ? QStringLiteral("—") : row.unit;
    case LimitsRole: {
        const QString min = row.minimum.isEmpty() ? QStringLiteral("—")
                                                  : slx::displayNumber(row.minimum);
        const QString max = row.maximum.isEmpty() ? QStringLiteral("—")
                                                  : slx::displayNumber(row.maximum);
        return min + QStringLiteral(" / ") + max;
    }
    case OutcomeRole: return slx::outcomeText(row.outcome);
    case OutcomeCodeRole:
        return row.outcome == slx::Outcome::Passed ? QStringLiteral("pass")
             : row.outcome == slx::Outcome::Failed ? QStringLiteral("fail")
             : row.outcome == slx::Outcome::Skipped ? QStringLiteral("skipped")
                                                    : QStringLiteral("invalid");
    case DetailRole: return row.detail;
    case LineRole: return row.line;
    default: return {};
    }
}

/// @brief Declares the stable role names expected by QML result delegates.
QHash<int, QByteArray> ResultsModel::roleNames() const
{
    return {{TestNameRole, "testName"}, {MeasurementRole, "measurement"},
            {UnitRole, "unit"}, {LimitsRole, "limits"}, {OutcomeRole, "outcome"},
            {OutcomeCodeRole, "outcomeCode"}, {DetailRole, "detail"}, {LineRole, "line"}};
}

/// @brief Replaces the selected device's rows in one model reset.
void ResultsModel::setResults(QVector<slx::TestResult> rows)
{
    beginResetModel();
    m_rows = std::move(rows);
    endResetModel();
}
