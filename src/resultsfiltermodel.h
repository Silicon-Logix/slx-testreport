// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

/// @file
/// @brief Declares the view-only result filter consumed by QML.

#pragma once

#include <QSortFilterProxyModel>

/// Filters the visible table without changing the source used for PDF export.
class ResultsFilterModel final : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(QString status READ status WRITE setStatus NOTIFY statusChanged)

public:
    /// @brief Creates an outcome and text filter over a separate source model.
    explicit ResultsFilterModel(QObject *parent = nullptr);

    /// @brief Returns the normalized case-insensitive search text.
    QString query() const { return m_query; }
    /// @brief Returns the active outcome code or all for an unfiltered view.
    QString status() const { return m_status; }
    /// @brief Reevaluates visible rows when the normalized search text changes.
    void setQuery(const QString &query);
    /// @brief Reevaluates visible rows when the outcome criterion changes.
    void setStatus(const QString &status);

signals:
    /// @brief Notifies QML that search text changed.
    void queryChanged();
    /// @brief Notifies QML that the outcome criterion changed.
    void statusChanged();

protected:
    /// @brief Combines the outcome criterion with a search over display evidence.
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QString m_query;
    QString m_status = QStringLiteral("all");
};
