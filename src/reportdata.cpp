// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

/// @file
/// @brief Parses bench CSV and classifies numeric and functional results.

#include "reportdata.h"

#include <QHash>
#include <QStringList>

#include <cmath>
#include <utility>

namespace slx {
namespace {

struct CsvRecord {
    QStringList fields;
    int line = 1;
    bool valid = true;
};

/// @brief Selects comma or semicolon from the first nonblank header line while ignoring
/// quoted punctuation.
QChar detectDelimiter(const QString &text)
{
    int start = 0;
    while (start < text.size()) {
        const int end = text.indexOf(QLatin1Char('\n'), start);
        const int length = (end < 0 ? text.size() : end) - start;
        if (!text.mid(start, length).trimmed().isEmpty()) break;
        if (end < 0) break;
        start = end + 1;
    }
    int commas = 0;
    int semicolons = 0;
    bool quoted = false;
    for (int i = start; i < text.size(); ++i) {
        const QChar ch = text.at(i);
        if (ch == QLatin1Char('"')) {
            if (quoted && i + 1 < text.size() && text.at(i + 1) == QLatin1Char('"')) {
                ++i;
            } else {
                quoted = !quoted;
            }
        } else if (!quoted && ch == QLatin1Char('\n')) {
            break;
        } else if (!quoted && ch == QLatin1Char(',')) {
            ++commas;
        } else if (!quoted && ch == QLatin1Char(';')) {
            ++semicolons;
        }
    }
    return semicolons > commas ? QLatin1Char(';') : QLatin1Char(',');
}

/// @brief Splits CSV records without losing quoted newlines or their physical source
/// line.
QVector<CsvRecord> tokenize(QString text, QChar delimiter)
{
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QLatin1Char('\r'), QLatin1Char('\n'));

    enum class State { Start, Plain, Quoted, AfterQuote };
    State state = State::Start;
    QVector<CsvRecord> records;
    CsvRecord record;
    QString field;
    bool hasContent = false;
    int line = 1;

    // Complete a cell and reset the tokenizer for the next column.
    auto finishField = [&] {
        record.fields.append(field);
        field.clear();
        state = State::Start;
    };
    // Store a nonblank record with its starting physical line.
    auto finishRecord = [&] {
        finishField();
        if (hasContent || record.fields.size() > 1 || !record.fields.first().trimmed().isEmpty()) {
            records.append(record);
        }
        record = CsvRecord{};
        record.line = line + 1;
        hasContent = false;
    };

    for (int i = 0; i < text.size(); ++i) {
        const QChar ch = text.at(i);
        if (state == State::Quoted) {
            if (ch == QLatin1Char('"')) {
                if (i + 1 < text.size() && text.at(i + 1) == QLatin1Char('"')) {
                    field += ch;
                    ++i;
                } else {
                    state = State::AfterQuote;
                }
            } else {
                field += ch;
                if (ch == QLatin1Char('\n')) {
                    ++line;
                }
            }
            continue;
        }
        if (ch == delimiter) {
            finishField();
            hasContent = true;
        } else if (ch == QLatin1Char('\n')) {
            finishRecord();
            ++line;
        } else if (ch == QLatin1Char('"')) {
            if (state == State::Start) {
                state = State::Quoted;
                hasContent = true;
            } else {
                record.valid = false;
                field += ch;
            }
        } else if (state == State::AfterQuote) {
            if (!ch.isSpace()) {
                record.valid = false;
                field += ch;
            }
        } else {
            state = State::Plain;
            field += ch;
            hasContent = true;
        }
    }
    if (state == State::Quoted) {
        record.valid = false;
    }
    if (hasContent || !field.isEmpty() || !record.fields.isEmpty()) {
        finishRecord();
    }
    return records;
}

/// @brief Maps only recognized Italian and English column names to the report schema.
QString canonicalHeader(QString header)
{
    header = header.trimmed().toLower();
    header.remove(QLatin1Char(' '));
    header.replace(QLatin1Char('-'), QLatin1Char('_'));
    if (header == QStringLiteral("matricola") || header == QStringLiteral("serial_number")) return QStringLiteral("serial");
    if (header == QStringLiteral("prova") || header == QStringLiteral("test_name")) return QStringLiteral("test");
    if (header == QStringLiteral("misura") || header == QStringLiteral("measurement")) return QStringLiteral("value");
    if (header == QStringLiteral("unita") || header == QStringLiteral("unità")) return QStringLiteral("unit");
    if (header == QStringLiteral("limite_min") || header == QStringLiteral("minimum")) return QStringLiteral("min");
    if (header == QStringLiteral("limite_max") || header == QStringLiteral("maximum")) return QStringLiteral("max");
    if (header == QStringLiteral("esito") || header == QStringLiteral("result")
        || header == QStringLiteral("verdict")) return QStringLiteral("status");
    if (header == QStringLiteral("diagnostica") || header == QStringLiteral("detail")
        || header == QStringLiteral("message")) return QStringLiteral("diagnostic");
    return header;
}

} // namespace

/// @brief Parses a finite decimal independently of the OS locale and rejects mixed
/// separators.
bool parseNumber(QString text, double &value)
{
    text = text.trimmed();
    if (text.isEmpty() || (text.contains(QLatin1Char('.')) && text.contains(QLatin1Char(',')))
        || text.count(QLatin1Char(',')) > 1) {
        return false;
    }
    text.replace(QLatin1Char(','), QLatin1Char('.'));
    bool ok = false;
    value = text.toDouble(&ok);
    return ok && std::isfinite(value);
}

/// @brief Normalizes only the decimal mark of a valid source value, preserving its precision.
QString displayNumber(QString text)
{
    double value = 0.0;
    if (parseNumber(text, value)) text.replace(QLatin1Char(','), QLatin1Char('.'));
    return text;
}

/// @brief Maps supported bench status spellings to a stable declared-status value.
DeclaredStatus parseStatus(QString text)
{
    text = text.trimmed().toUpper();
    text.replace(QLatin1Char(' '), QLatin1Char('_'));
    text.replace(QLatin1Char('-'), QLatin1Char('_'));
    if (text.isEmpty()) return DeclaredStatus::None;
    if (text == QStringLiteral("PASS") || text == QStringLiteral("PASSED")
        || text == QStringLiteral("OK")) return DeclaredStatus::Pass;
    if (text == QStringLiteral("FAIL") || text == QStringLiteral("FAILED")
        || text == QStringLiteral("NOK") || text == QStringLiteral("NG")) return DeclaredStatus::Fail;
    if (text == QStringLiteral("SKIP") || text == QStringLiteral("SKIPPED")
        || text == QStringLiteral("NOT_RUN") || text == QStringLiteral("NOT_TESTED")) return DeclaredStatus::Skip;
    if (text == QStringLiteral("N/A") || text == QStringLiteral("NA")
        || text == QStringLiteral("NOT_APPLICABLE")) return DeclaredStatus::Skip;
    if (text == QStringLiteral("ERROR") || text == QStringLiteral("ERR")
        || text == QStringLiteral("ABORTED") || text == QStringLiteral("INCONCLUSIVE")) return DeclaredStatus::Error;
    return DeclaredStatus::Unknown;
}

/// @brief Provides the English display label for a classified result.
QString outcomeText(Outcome outcome)
{
    switch (outcome) {
    case Outcome::Passed: return QStringLiteral("Pass");
    case Outcome::Failed: return QStringLiteral("Fail");
    case Outcome::Skipped: return QStringLiteral("Skipped");
    case Outcome::Invalid: return QStringLiteral("Invalid data");
    }
    return {};
}

/// @brief Rejects structural CSV errors while retaining malformed measurements as
/// traceable Invalid rows.
ImportResult parseCsv(const QByteArray &contents)
{
    ImportResult result;
    QByteArray utf8 = contents;
    if (utf8.startsWith("\xEF\xBB\xBF")) {
        utf8.remove(0, 3);
    }
    const QString text = QString::fromUtf8(utf8);
    if (text.toUtf8() != utf8) {
        result.error = QStringLiteral("The CSV must be valid UTF-8.");
        return result;
    }
    if (text.trimmed().isEmpty()) {
        result.error = QStringLiteral("The CSV file is empty.");
        return result;
    }

    const auto records = tokenize(text, detectDelimiter(text));
    if (records.isEmpty() || !records.first().valid) {
        result.error = QStringLiteral("Invalid CSV header.");
        return result;
    }

    const QStringList known = {QStringLiteral("serial"), QStringLiteral("test"),
                               QStringLiteral("value"), QStringLiteral("unit"),
                               QStringLiteral("min"), QStringLiteral("max"),
                               QStringLiteral("status"), QStringLiteral("diagnostic")};
    QHash<QString, int> columns;
    const auto &headers = records.first().fields;
    for (int i = 0; i < headers.size(); ++i) {
        const QString key = canonicalHeader(headers.at(i));
        if (known.contains(key)) {
            if (columns.contains(key)) {
                result.error = QStringLiteral("Duplicate CSV column: %1.").arg(key);
                return result;
            }
            columns.insert(key, i);
        } else if (!headers.at(i).trimmed().isEmpty()) {
            result.ignoredColumns.append(headers.at(i).trimmed());
        }
    }
    for (const auto &key : {QStringLiteral("serial"), QStringLiteral("test")}) {
        if (!columns.contains(key)) {
            result.error = QStringLiteral("Missing CSV column: %1.").arg(key);
            return result;
        }
    }
    if (!columns.contains(QStringLiteral("value")) && !columns.contains(QStringLiteral("status"))) {
        result.error = QStringLiteral("The CSV needs a value or status column.");
        return result;
    }
    if (records.size() == 1) {
        result.error = QStringLiteral("The CSV contains no tests.");
        return result;
    }

    // Missing optional columns and short records produce an empty cell.
    auto fieldAt = [&](const CsvRecord &record, const QString &key) {
        if (!columns.contains(key)) return QString{};
        const int index = columns.value(key);
        return index < record.fields.size() ? record.fields.at(index).trimmed() : QString{};
    };

    for (int i = 1; i < records.size(); ++i) {
        const auto &record = records.at(i);
        TestResult row;
        row.line = record.line;
        row.serial = fieldAt(record, QStringLiteral("serial"));
        row.testName = fieldAt(record, QStringLiteral("test"));
        row.measurement = fieldAt(record, QStringLiteral("value"));
        row.unit = fieldAt(record, QStringLiteral("unit"));
        row.minimum = fieldAt(record, QStringLiteral("min"));
        row.maximum = fieldAt(record, QStringLiteral("max"));
        row.reportedStatus = fieldAt(record, QStringLiteral("status"));
        row.diagnostic = fieldAt(record, QStringLiteral("diagnostic"));
        row.recordValid = record.valid && record.fields.size() == headers.size();
        const DeclaredStatus declared = parseStatus(row.reportedStatus);

        if (result.serial.isEmpty() && !row.serial.isEmpty()) {
            result.serial = row.serial;
        }
        double measured = 0.0, lower = 0.0, upper = 0.0;
        if (!row.recordValid) {
            row.detail = QStringLiteral("Malformed CSV record");
        } else if (row.serial.isEmpty()) {
            row.detail = QStringLiteral("Missing serial number");
        } else if (row.testName.isEmpty()) {
            row.detail = QStringLiteral("Missing test name");
        } else if (declared == DeclaredStatus::Unknown) {
            row.detail = QStringLiteral("Unsupported result status: %1").arg(row.reportedStatus);
        } else if (declared == DeclaredStatus::Error) {
            row.detail = QStringLiteral("Bench reported an error");
        } else if (declared == DeclaredStatus::Skip) {
            row.outcome = Outcome::Skipped;
            if (row.diagnostic.isEmpty()) row.detail = QStringLiteral("Not executed");
        } else if (row.minimum.isEmpty() && row.maximum.isEmpty()) {
            if (declared == DeclaredStatus::Pass) {
                row.outcome = Outcome::Passed;
            } else if (declared == DeclaredStatus::Fail) {
                row.outcome = Outcome::Failed;
                if (row.diagnostic.isEmpty()) row.detail = QStringLiteral("Functional check failed");
            } else {
                row.detail = QStringLiteral("No limits or result status");
            }
        } else if (row.measurement.isEmpty()) {
            row.detail = QStringLiteral("Missing measurement");
        } else if (!parseNumber(row.measurement, measured)) {
            row.detail = QStringLiteral("Non-numeric measurement");
        } else if (!row.minimum.isEmpty() && !parseNumber(row.minimum, lower)) {
            row.detail = QStringLiteral("Invalid lower limit");
        } else if (!row.maximum.isEmpty() && !parseNumber(row.maximum, upper)) {
            row.detail = QStringLiteral("Invalid upper limit");
        } else if (!row.minimum.isEmpty() && !row.maximum.isEmpty() && lower > upper) {
            row.detail = QStringLiteral("Lower limit exceeds upper limit");
        } else {
            const bool below = !row.minimum.isEmpty() && measured < lower;
            const bool above = !row.maximum.isEmpty() && measured > upper;
            const bool measuredPass = !below && !above;
            row.outcome = measuredPass ? Outcome::Passed : Outcome::Failed;
            if (below) row.detail = QStringLiteral("Below lower limit");
            if (above) row.detail = QStringLiteral("Above upper limit");
            if ((declared == DeclaredStatus::Pass && !measuredPass)
                || (declared == DeclaredStatus::Fail && measuredPass)) {
                row.outcome = Outcome::Invalid;
                row.detail = QStringLiteral("Reported status conflicts with measured limits");
            }
            if (!row.minimum.isEmpty() && !row.maximum.isEmpty() && upper > lower) {
                const double span = upper - lower;
                const double position = (measured - lower) / span;
                if (std::isfinite(span) && std::isfinite(position)) {
                    row.hasTwoSidedRange = true;
                    row.rangePosition = position;
                }
            }
        }
        if (!row.diagnostic.isEmpty()) {
            row.detail = row.detail.isEmpty() ? row.diagnostic : row.detail + QStringLiteral(" · ") + row.diagnostic;
        }
        result.rows.append(std::move(row));
    }
    return result;
}

} // namespace slx
