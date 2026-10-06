// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

/// @file
/// @brief Defines the independent acceptance profile for a device run.

#pragma once

#include "reportdata.h"

#include <QVector>

namespace slx {

enum class TestKind { Numeric, Functional };

struct TestDefinition {
    QString name;
    TestKind kind = TestKind::Numeric;
    QString unit;
    QString minimum;
    QString maximum;
    double lower = 0.0;
    double upper = 0.0;
    bool hasLower = false;
    bool hasUpper = false;
    bool required = true;
};

/// Stores acceptance criteria separately from the measurements they evaluate.
struct TestProfile {
    QString model;
    QString revision;
    QString fileName;
    QString hash;
    QVector<TestDefinition> tests;
};

struct ProfileResult {
    TestProfile profile;
    QString error;
};

/// @brief Parses a versioned test plan; an error leaves its definitions unusable.
/// @return A complete profile when error is empty; otherwise a diagnostic.
ProfileResult parseProfileJson(const QByteArray &contents);

/// @brief Evaluates one serial against independent criteria and retains missing,
/// duplicate, or unexpected tests as rows.
/// @param source Original CSV rows, which are not modified.
/// @param serial Device whose expected tests are being evaluated.
QVector<TestResult> applyProfile(const QVector<TestResult> &source,
                                const TestProfile &profile, const QString &serial);

} // namespace slx
