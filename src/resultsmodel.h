// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

/// @file
/// @brief Declares the selected device's read-only result model.

#pragma once

#include "reportdata.h"

#include <QAbstractListModel>

/// Presents one device's complete results through stable roles used by QML.
class ResultsModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        TestNameRole = Qt::UserRole + 1,
        MeasurementRole,
        UnitRole,
        LimitsRole,
        OutcomeRole,
        OutcomeCodeRole,
        DetailRole,
        LineRole
    };

    /// @brief Creates the read-only model used by the selected-device table.
    explicit ResultsModel(QObject *parent = nullptr);
    /// @brief Reports top-level result rows; child indexes have no rows.
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    /// @brief Returns a display role or an empty value for invalid indexes.
    QVariant data(const QModelIndex &index, int role) const override;
    /// @brief Defines the stable role names consumed by QML delegates.
    QHash<int, QByteArray> roleNames() const override;

    /// @brief Replaces all visible device rows in one model reset.
    void setResults(QVector<slx::TestResult> rows);
    /// @brief Returns the complete evaluated rows of the selected device.
    const QVector<slx::TestResult> &results() const { return m_rows; }

private:
    QVector<slx::TestResult> m_rows;
};
