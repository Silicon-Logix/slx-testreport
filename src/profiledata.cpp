// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

/// @file
/// @brief Validates test plans and evaluates source readings against them.

#include "profiledata.h"

#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>

#include <algorithm>
#include <cmath>

namespace slx {
namespace {

/// @brief Normalizes a test name for case-insensitive profile matching.
QString keyFor(const QString &name)
{
    return name.trimmed().toCaseFolded();
}

/// @brief Serializes a profile limit with enough precision for diagnostic text.
QString numberText(double value)
{
    return QString::number(value, 'g', 16);
}

/// @brief Treats tiny floating-point representation differences as equal in limit
/// comparisons.
bool sameNumber(double a, double b)
{
    return std::abs(a - b) <= 1e-12 * std::max({1.0, std::abs(a), std::abs(b)});
}

/// @brief Joins an evaluation reason with the original bench diagnostic without losing
/// either.
QString withDiagnostic(const QString &reason, const QString &diagnostic)
{
    return diagnostic.isEmpty() ? reason
        : reason.isEmpty() ? diagnostic : reason + QStringLiteral(" · ") + diagnostic;
}

/// @brief Checks an exposed CSV limit against the profile; an absent CSV limit is
/// acceptable.
bool csvLimitMatches(const QString &actual, bool expectedPresent, double expected)
{
    if (actual.isEmpty()) return true;
    double parsed = 0.0;
    return expectedPresent && parseNumber(actual, parsed) && sameNumber(parsed, expected);
}

} // namespace

/// @brief Validates the versioned test plan and returns either complete definitions or
/// a diagnostic.
ProfileResult parseProfileJson(const QByteArray &contents)
{
    ProfileResult result;
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(contents, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        result.error = parseError.error != QJsonParseError::NoError
            ? QStringLiteral("Invalid test profile JSON: %1").arg(parseError.errorString())
            : QStringLiteral("The test profile root must be a JSON object.");
        return result;
    }
    const QJsonObject root = document.object();
    if (!root.value(QStringLiteral("schemaVersion")).isDouble()
        || root.value(QStringLiteral("schemaVersion")).toDouble() != 1.0) {
        result.error = QStringLiteral("Unsupported test profile schema version.");
        return result;
    }
    result.profile.model = root.value(QStringLiteral("model")).toString().trimmed();
    result.profile.revision = root.value(QStringLiteral("revision")).toString().trimmed();
    if (result.profile.model.isEmpty() || result.profile.revision.isEmpty()) {
        result.error = QStringLiteral("The profile needs a model and revision.");
        return result;
    }
    const QJsonValue testsValue = root.value(QStringLiteral("tests"));
    if (!testsValue.isArray() || testsValue.toArray().isEmpty()) {
        result.error = QStringLiteral("The profile needs at least one test.");
        return result;
    }
    QSet<QString> names;
    const QJsonArray tests = testsValue.toArray();
    for (int i = 0; i < tests.size(); ++i) {
        if (!tests.at(i).isObject()) {
            result.error = QStringLiteral("Profile test %1 must be an object.").arg(i + 1);
            return result;
        }
        const QJsonObject object = tests.at(i).toObject();
        TestDefinition definition;
        definition.name = object.value(QStringLiteral("name")).toString().trimmed();
        const QString kind = object.value(QStringLiteral("kind")).toString().trimmed().toLower();
        if (definition.name.isEmpty() || names.contains(keyFor(definition.name))) {
            result.error = QStringLiteral("Empty or duplicate test name at profile entry %1.").arg(i + 1);
            return result;
        }
        names.insert(keyFor(definition.name));
        if (kind == QStringLiteral("numeric")) definition.kind = TestKind::Numeric;
        else if (kind == QStringLiteral("functional")) definition.kind = TestKind::Functional;
        else {
            result.error = QStringLiteral("Unsupported kind for profile test %1.").arg(definition.name);
            return result;
        }
        if (object.contains(QStringLiteral("unit")) && !object.value(QStringLiteral("unit")).isString()) {
            result.error = QStringLiteral("Invalid unit for profile test %1.").arg(definition.name);
            return result;
        }
        definition.unit = object.value(QStringLiteral("unit")).toString().trimmed();
        if (object.contains(QStringLiteral("required"))) {
            if (!object.value(QStringLiteral("required")).isBool()) {
                result.error = QStringLiteral("Invalid required flag for profile test %1.").arg(definition.name);
                return result;
            }
            definition.required = object.value(QStringLiteral("required")).toBool();
        }
        definition.hasLower = object.contains(QStringLiteral("min"));
        definition.hasUpper = object.contains(QStringLiteral("max"));
        if (definition.kind == TestKind::Numeric) {
            if ((!definition.hasLower && !definition.hasUpper)
                || (definition.hasLower && !object.value(QStringLiteral("min")).isDouble())
                || (definition.hasUpper && !object.value(QStringLiteral("max")).isDouble())) {
                result.error = QStringLiteral("Numeric profile test %1 needs valid limits.").arg(definition.name);
                return result;
            }
            if (definition.hasLower) {
                definition.lower = object.value(QStringLiteral("min")).toDouble();
                definition.minimum = numberText(definition.lower);
            }
            if (definition.hasUpper) {
                definition.upper = object.value(QStringLiteral("max")).toDouble();
                definition.maximum = numberText(definition.upper);
            }
            if ((definition.hasLower && !std::isfinite(definition.lower))
                || (definition.hasUpper && !std::isfinite(definition.upper))
                || (definition.hasLower && definition.hasUpper && definition.lower > definition.upper)) {
                result.error = QStringLiteral("Inconsistent limits for profile test %1.").arg(definition.name);
                return result;
            }
        } else if (definition.hasLower || definition.hasUpper || !definition.unit.isEmpty()) {
            result.error = QStringLiteral("Functional profile test %1 cannot declare numeric limits or a unit.")
                               .arg(definition.name);
            return result;
        }
        result.profile.tests.append(definition);
    }
    return result;
}

/// @brief Reevaluates one device against independent limits while preserving source
/// rows and surfacing missing or unexpected tests.
QVector<TestResult> applyProfile(const QVector<TestResult> &source,
                                const TestProfile &profile, const QString &serial)
{
    QHash<QString, const TestDefinition *> definitions;
    for (const auto &definition : profile.tests) definitions.insert(keyFor(definition.name), &definition);

    QVector<TestResult> evaluated;
    QHash<QString, int> seen;
    evaluated.reserve(source.size() + profile.tests.size());
    for (const auto &original : source) {
        TestResult row = original;
        row.hasTwoSidedRange = false;
        row.rangePosition = 0.0;
        const QString key = keyFor(row.testName);
        if (!key.isEmpty() && seen.contains(key)) {
            auto &previous = evaluated[seen.value(key)];
            previous.outcome = Outcome::Invalid;
            previous.hasTwoSidedRange = false;
            previous.detail = withDiagnostic(QStringLiteral("Duplicate test in device data"), previous.diagnostic);
            row.outcome = Outcome::Invalid;
            row.detail = withDiagnostic(QStringLiteral("Duplicate test in device data"), row.diagnostic);
            evaluated.append(row);
            continue;
        }
        if (!key.isEmpty()) seen.insert(key, evaluated.size());
        if (!row.recordValid || row.serial.isEmpty() || row.testName.isEmpty()) {
            evaluated.append(row);
            continue;
        }
        const auto found = definitions.constFind(key);
        if (found == definitions.cend()) {
            row.outcome = Outcome::Invalid;
            row.detail = withDiagnostic(QStringLiteral("Test absent from profile"), row.diagnostic);
            evaluated.append(row);
            continue;
        }
        const TestDefinition &definition = **found;
        const QString csvMinimum = row.minimum;
        const QString csvMaximum = row.maximum;
        row.minimum = definition.minimum;
        row.maximum = definition.maximum;
        QString problem;
        if (definition.kind == TestKind::Functional) {
            if (!row.measurement.isEmpty() || !csvMinimum.isEmpty()
                || !csvMaximum.isEmpty() || !row.unit.isEmpty()) {
                problem = QStringLiteral("Functional test has unexpected measurement, limits or unit");
            }
        } else {
            if ((!row.unit.isEmpty() || !row.measurement.isEmpty()) && row.unit != definition.unit) {
                problem = QStringLiteral("CSV unit differs from profile (%1 expected)").arg(
                    definition.unit.isEmpty() ? QStringLiteral("dimensionless") : definition.unit);
            }
            if (!csvLimitMatches(csvMinimum, definition.hasLower, definition.lower)
                || !csvLimitMatches(csvMaximum, definition.hasUpper, definition.upper)) {
                if (!problem.isEmpty()) problem += QStringLiteral("; ");
                problem += QStringLiteral("CSV limits differ from profile (%1 / %2 in source)")
                               .arg(csvMinimum.isEmpty() ? QStringLiteral("—") : csvMinimum,
                                    csvMaximum.isEmpty() ? QStringLiteral("—") : csvMaximum);
            }
        }
        if (definition.kind == TestKind::Numeric
            && row.unit.isEmpty() && row.measurement.isEmpty()) {
            row.unit = definition.unit;
        }
        const DeclaredStatus declared = parseStatus(row.reportedStatus);
        if (problem.isEmpty() && declared == DeclaredStatus::Unknown)
            problem = QStringLiteral("Unsupported result status: %1").arg(row.reportedStatus);
        if (!problem.isEmpty()) {
            row.outcome = Outcome::Invalid;
        } else if (declared == DeclaredStatus::Error) {
            row.outcome = Outcome::Invalid;
            problem = QStringLiteral("Bench reported an error");
        } else if (declared == DeclaredStatus::Skip) {
            row.outcome = Outcome::Skipped;
            problem = QStringLiteral("Not executed");
        } else if (definition.kind == TestKind::Functional) {
            if (declared == DeclaredStatus::Pass) row.outcome = Outcome::Passed;
            else if (declared == DeclaredStatus::Fail) {
                row.outcome = Outcome::Failed;
                problem = QStringLiteral("Functional check failed");
            } else {
                row.outcome = Outcome::Invalid;
                problem = QStringLiteral("Functional check needs a declared status");
            }
        } else {
            double measured = 0.0;
            if (row.measurement.isEmpty() || !parseNumber(row.measurement, measured)) {
                row.outcome = Outcome::Invalid;
                problem = row.measurement.isEmpty() ? QStringLiteral("Missing measurement")
                                                    : QStringLiteral("Non-numeric measurement");
            } else {
                const bool below = definition.hasLower && measured < definition.lower;
                const bool above = definition.hasUpper && measured > definition.upper;
                const bool measuredPass = !below && !above;
                row.outcome = measuredPass ? Outcome::Passed : Outcome::Failed;
                if (below) problem = QStringLiteral("Below profile lower limit");
                if (above) problem = QStringLiteral("Above profile upper limit");
                if ((declared == DeclaredStatus::Pass && !measuredPass)
                    || (declared == DeclaredStatus::Fail && measuredPass)) {
                    row.outcome = Outcome::Invalid;
                    problem = QStringLiteral("Reported status conflicts with profile limits");
                }
                if (definition.hasLower && definition.hasUpper && definition.upper > definition.lower) {
                    const double span = definition.upper - definition.lower;
                    const double position = (measured - definition.lower) / span;
                    if (std::isfinite(span) && std::isfinite(position)) {
                        row.hasTwoSidedRange = true;
                        row.rangePosition = position;
                    }
                }
            }
        }
        row.detail = withDiagnostic(problem, row.diagnostic);
        evaluated.append(row);
    }
    for (const auto &definition : profile.tests) {
        if (!definition.required || seen.contains(keyFor(definition.name))) continue;
        TestResult missing;
        missing.line = 0;
        missing.serial = serial;
        missing.testName = definition.name;
        missing.unit = definition.unit;
        missing.minimum = definition.minimum;
        missing.maximum = definition.maximum;
        missing.detail = QStringLiteral("Required test absent from CSV");
        evaluated.append(missing);
    }
    return evaluated;
}

} // namespace slx
