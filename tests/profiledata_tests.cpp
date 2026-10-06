// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

/// @file
/// @brief Checks plan validation and independent acceptance verdicts.

#include "../src/profiledata.h"

#include <QtTest>

class ProfileDataTests final : public QObject
{
    Q_OBJECT

private slots:
    /// @brief Rejects invalid schema, duplicate definitions, and reversed numeric limits.
    void rejectsBadProfiles()
    {
        const auto duplicate = slx::parseProfileJson(
            R"({"schemaVersion":1,"model":"M","revision":"A","tests":[
                {"name":"Voltage","kind":"numeric","min":0},
                {"name":" voltage ","kind":"functional"}]})");
        QVERIFY(duplicate.error.contains(QStringLiteral("duplicate")));
        const auto reversed = slx::parseProfileJson(
            R"({"schemaVersion":1,"model":"M","revision":"A","tests":[
                {"name":"Voltage","kind":"numeric","min":10,"max":5}]})");
        QVERIFY(reversed.error.contains(QStringLiteral("Inconsistent")));
        const auto wrongSchema = slx::parseProfileJson(
            R"({"schemaVersion":2,"model":"M","revision":"A","tests":[]})");
        QVERIFY(wrongSchema.error.contains(QStringLiteral("schema")));
    }

    /// @brief Keeps plan omissions and CSV/profile disagreements visible as report
    /// evidence.
    void findsMissingDuplicateUnexpectedAndDisputedLimits()
    {
        const auto profile = slx::parseProfileJson(
            R"({"schemaVersion":1,"model":"M","revision":"B","tests":[
                {"name":"Voltage","kind":"numeric","unit":"V","min":4.9,"max":5.1},
                {"name":"Current","kind":"numeric","unit":"A","max":0.3},
                {"name":"Barcode","kind":"functional"},
                {"name":"Optional","kind":"functional","required":false}]})");
        QVERIFY(profile.error.isEmpty());
        const auto source = slx::parseCsv(
            "serial;test;value;unit;min;max;status\n"
            "A1;Voltage;5.0;V;;;\n"
            "A1;Current;0.2;A;;0.25;\n"
            "A1;Barcode;;;;;PASS\n"
            "A1;Barcode;;;;;PASS\n"
            "A1;Mystery;;;;;PASS\n");
        QVERIFY(source.error.isEmpty());
        const auto rows = slx::applyProfile(source.rows, profile.profile, QStringLiteral("A1"));
        QCOMPARE(rows.size(), 5);
        QCOMPARE(rows.at(0).outcome, slx::Outcome::Passed);
        QCOMPARE(rows.at(0).minimum, QStringLiteral("4.9"));
        QCOMPARE(rows.at(1).outcome, slx::Outcome::Invalid);
        QVERIFY(rows.at(1).detail.contains(QStringLiteral("limits differ")));
        QCOMPARE(rows.at(2).outcome, slx::Outcome::Invalid);
        QCOMPARE(rows.at(3).outcome, slx::Outcome::Invalid);
        QVERIFY(rows.at(3).detail.contains(QStringLiteral("Duplicate")));
        QCOMPARE(rows.at(4).outcome, slx::Outcome::Invalid);
        QVERIFY(rows.at(4).detail.contains(QStringLiteral("absent from profile")));
    }

    /// @brief Creates missing-test rows only for requirements marked mandatory.
    void addsOnlyRequiredMissingTests()
    {
        const auto profile = slx::parseProfileJson(
            R"({"schemaVersion":1,"model":"M","revision":"A","tests":[
                {"name":"Voltage","kind":"numeric","unit":"V","min":4.9,"max":5.1},
                {"name":"LED","kind":"functional"},
                {"name":"Optional","kind":"functional","required":false}]})");
        QVERIFY(profile.error.isEmpty());
        const auto source = slx::parseCsv(
            "serial;test;value;unit\nA1;Voltage;5.0;V\n");
        QVERIFY(source.error.isEmpty());
        const auto rows = slx::applyProfile(source.rows, profile.profile, QStringLiteral("A1"));
        QCOMPARE(rows.size(), 2);
        QCOMPARE(rows.at(0).outcome, slx::Outcome::Passed);
        QCOMPARE(rows.at(1).testName, QStringLiteral("LED"));
        QCOMPARE(rows.at(1).line, 0);
        QCOMPARE(rows.at(1).outcome, slx::Outcome::Invalid);
    }

    /// @brief Rejects a numeric reading when a profile requires a functional verdict.
    void rejectsMeasurementOnFunctionalCheck()
    {
        const auto profile = slx::parseProfileJson(
            R"({"schemaVersion":1,"model":"M","revision":"A","tests":[
                {"name":"Barcode","kind":"functional"}]})");
        QVERIFY(profile.error.isEmpty());
        const auto source = slx::parseCsv(
            "serial;test;value;status\nA1;Barcode;42;PASS\n");
        QVERIFY(source.error.isEmpty());
        const auto rows = slx::applyProfile(source.rows, profile.profile, QStringLiteral("A1"));
        QCOMPARE(rows.size(), 1);
        QCOMPARE(rows.at(0).outcome, slx::Outcome::Invalid);
        QVERIFY(rows.at(0).detail.contains(QStringLiteral("unexpected measurement")));
    }
};

QTEST_GUILESS_MAIN(ProfileDataTests)
#include "profiledata_tests.moc"
