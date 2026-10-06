// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

/// @file
/// @brief Defines the report metadata and PDF publication contract.

#pragma once

#include "reportdata.h"

#include <QString>

namespace slx {

/// Captures the identity and provenance printed with a device's measurements.
struct ReportMetadata {
    QString model;
    QString serial;
    QString date;
    QString operatorName;
    QString stationId;
    QString procedureRevision;
    QString reportId;
    QString importedAt;
    QString sourceFile;
    QString sourceHash;
    QString profileFile;
    QString profileHash;
    QString overallOutcome;
    int passed = 0;
    int failed = 0;
    int skipped = 0;
    int invalid = 0;
};

/// @brief Writes all device results to an A4 PDF and atomically replaces the target;
/// returns false with an error on failure.
/// @param[out] error Receives a diagnostic on failure and is cleared on success.
/// @return True only after the complete file has been committed.
bool writePdf(const QString &path, const ReportMetadata &metadata,
              const QVector<TestResult> &rows, QString &error);

} // namespace slx
