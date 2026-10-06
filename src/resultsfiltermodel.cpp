// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

/// @file
/// @brief Filters the visible result table without changing report evidence.

#include "resultsfiltermodel.h"

#include "resultsmodel.h"

/// @brief Creates a proxy that leaves the underlying evidence model owned by its
/// caller.
ResultsFilterModel::ResultsFilterModel(QObject *parent) : QSortFilterProxyModel(parent) {}

/// @brief Normalizes search text and invalidates visible rows only when the query
/// changes.
void ResultsFilterModel::setQuery(const QString &query)
{
    const QString normalized = query.trimmed();
    if (m_query == normalized) return;
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    beginFilterChange();
#endif
    m_query = normalized;
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
#else
    invalidateFilter();
#endif
    emit queryChanged();
}

/// @brief Changes the outcome filter without altering source rows or their order.
void ResultsFilterModel::setStatus(const QString &status)
{
    if (m_status == status) return;
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    beginFilterChange();
#endif
    m_status = status;
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
#else
    invalidateFilter();
#endif
    emit statusChanged();
}

/// @brief Combines recognized outcome filters with case-insensitive searches over
/// visible evidence fields.
bool ResultsFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    if (m_status != QStringLiteral("all")
        && m_status != sourceModel()->data(index, ResultsModel::OutcomeCodeRole).toString()) {
        if (m_status == QStringLiteral("pass") || m_status == QStringLiteral("fail")
            || m_status == QStringLiteral("skipped")
            || m_status == QStringLiteral("invalid")) return false;
    }
    if (m_query.isEmpty()) return true;
    const int roles[] = { ResultsModel::TestNameRole, ResultsModel::MeasurementRole,
                          ResultsModel::UnitRole, ResultsModel::LimitsRole,
                          ResultsModel::DetailRole };
    for (int role : roles) {
        if (sourceModel()->data(index, role).toString().contains(m_query, Qt::CaseInsensitive)) return true;
    }
    return false;
}
