// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

/// @file
/// @brief Coordinates imports, per-device state, profiles, and exports.

#include "reportcontroller.h"

#include "pdfexport.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QIODevice>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTemporaryDir>

#include <algorithm>
#include <utility>

namespace {

struct Counts { int passed = 0; int failed = 0; int skipped = 0; int invalid = 0; };

/// @brief Counts each verdict once for a device without filtering evidence rows.
Counts countOutcomes(const QVector<slx::TestResult> &rows)
{
    Counts counts;
    for (const auto &row : rows) {
        switch (row.outcome) {
        case slx::Outcome::Passed: ++counts.passed; break;
        case slx::Outcome::Failed: ++counts.failed; break;
        case slx::Outcome::Skipped: ++counts.skipped; break;
        case slx::Outcome::Invalid: ++counts.invalid; break;
        }
    }
    return counts;
}

/// @brief Combines failures, invalid data, and skipped checks into the run-level
/// verdict.
QString verdictFor(const Counts &counts, int total)
{
    if (total == 0) return QStringLiteral("NO DATA");
    if (counts.failed > 0) {
        QString verdict = QStringLiteral("FAIL");
        if (counts.invalid > 0) verdict += QStringLiteral(" · INVALID DATA");
        if (counts.skipped > 0) verdict += QStringLiteral(" · INCOMPLETE");
        return verdict;
    }
    if (counts.invalid > 0) return counts.skipped > 0
        ? QStringLiteral("REVIEW REQUIRED · INCOMPLETE") : QStringLiteral("REVIEW REQUIRED");
    if (counts.skipped > 0) return QStringLiteral("INCOMPLETE");
    return QStringLiteral("PASS");
}

/// @brief Replaces characters unsafe for generated report filenames with underscores.
QString safeFilePart(QString value)
{
    value.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_-]")), QStringLiteral("_"));
    return value;
}

/// @brief Ranks readings by distance to their own bounds and returns at most six
/// comparable highlights.
QVariantList buildToleranceHighlights(const QVector<slx::TestResult> &rows, int &boundedCount)
{
    struct Candidate { const slx::TestResult *row; double margin; };
    QVector<Candidate> candidates;
    for (const auto &row : rows) {
        if (!row.hasTwoSidedRange || row.outcome == slx::Outcome::Invalid
            || row.outcome == slx::Outcome::Skipped) continue;
        candidates.append({&row, std::min(row.rangePosition, 1.0 - row.rangePosition)});
    }
    boundedCount = candidates.size();
    // Equal margins retain CSV order so the shortlist is repeatable.
    std::stable_sort(candidates.begin(), candidates.end(),
                     [](const Candidate &a, const Candidate &b) { return a.margin < b.margin; });

    QVariantList highlights;
    const int shown = static_cast<int>(std::min<qsizetype>(6, candidates.size()));
    for (int i = 0; i < shown; ++i) {
        const auto &row = *candidates.at(i).row;
        QVariantMap item;
        item.insert(QStringLiteral("name"), row.testName);
        item.insert(QStringLiteral("value"), slx::displayNumber(row.measurement)
                                              + (row.unit.isEmpty() ? QString{} : QStringLiteral(" ") + row.unit));
        item.insert(QStringLiteral("minimum"), slx::displayNumber(row.minimum));
        item.insert(QStringLiteral("maximum"), slx::displayNumber(row.maximum));
        item.insert(QStringLiteral("position"), row.rangePosition);
        item.insert(QStringLiteral("failed"), row.outcome == slx::Outcome::Failed);
        item.insert(QStringLiteral("detail"), row.detail);
        highlights.append(item);
    }
    return highlights;
}

} // namespace

/// @brief Creates an empty report session with today's local test date.
ReportController::ReportController(QObject *parent)
    : QObject(parent), m_results(this), m_testDate(QDate::currentDate().toString(Qt::ISODate))
{
}

/// @brief Projects the first twelve results for the on-screen page without limiting PDF
/// export.
QVariantList ReportController::previewRows() const
{
    QVariantList preview;
    const auto &rows = m_results.results();
    const int shown = static_cast<int>(std::min<qsizetype>(12, rows.size()));
    for (int i = 0; i < shown; ++i) {
        const auto &row = rows.at(i);
        QVariantMap item;
        item.insert(QStringLiteral("name"), row.testName.isEmpty() ? QStringLiteral("Unnamed test") : row.testName);
        item.insert(QStringLiteral("value"), row.measurement.isEmpty() ? QStringLiteral("—")
                                                                  : slx::displayNumber(row.measurement));
        item.insert(QStringLiteral("unit"), row.unit);
        item.insert(QStringLiteral("limits"),
                    (row.minimum.isEmpty() ? QStringLiteral("—") : slx::displayNumber(row.minimum))
                    + QStringLiteral(" / ")
                    + (row.maximum.isEmpty() ? QStringLiteral("—") : slx::displayNumber(row.maximum)));
        item.insert(QStringLiteral("outcome"), slx::outcomeText(row.outcome));
        item.insert(QStringLiteral("code"), row.outcome == slx::Outcome::Passed ? QStringLiteral("pass")
                    : row.outcome == slx::Outcome::Failed ? QStringLiteral("fail")
                    : row.outcome == slx::Outcome::Skipped ? QStringLiteral("skipped")
                    : QStringLiteral("invalid"));
        preview.append(item);
    }
    return preview;
}

/// @brief Changes the model only when it is not fixed by an active test profile.
void ReportController::setModelName(const QString &value)
{
    if (m_profileActive && value != m_profile.model) return;
    if (m_modelName == value) return;
    m_modelName = value;
    emit metadataChanged();
}

/// @brief Stores an operator edit and notifies QML only when the value changes.
void ReportController::setOperatorName(const QString &value)
{
    if (m_operatorName == value) return;
    m_operatorName = value;
    emit metadataChanged();
}

/// @brief Stores a station edit and notifies QML only when the value changes.
void ReportController::setStationId(const QString &value)
{
    if (m_stationId == value) return;
    m_stationId = value;
    emit metadataChanged();
}

/// @brief Changes the procedure revision only when it is not fixed by an active
/// profile.
void ReportController::setProcedureRevision(const QString &value)
{
    if (m_profileActive && value != m_profile.revision) return;
    if (m_procedureRevision == value) return;
    m_procedureRevision = value;
    emit metadataChanged();
}

/// @brief Stores the selected test date; export validation checks its ISO format.
void ReportController::setTestDate(const QString &value)
{
    if (m_testDate == value) return;
    m_testDate = value;
    emit metadataChanged();
}

/// @brief Exposes the selected device's combined verdict, including simultaneous
/// failure and invalid data.
QString ReportController::overallOutcome() const
{
    return verdictFor({m_passedCount, m_failedCount, m_skippedCount, m_invalidCount}, totalCount());
}

/// @brief Builds the device selector entries from all isolated runs in CSV order.
QVariantList ReportController::devices() const
{
    QVariantList entries;
    for (const auto &device : m_devices) {
        const Counts counts = countOutcomes(device.rows);
        QVariantMap entry;
        entry.insert(QStringLiteral("serial"), device.serial);
        entry.insert(QStringLiteral("label"), QStringLiteral("%1 · %2")
                         .arg(device.serial, verdictFor(counts, device.rows.size())));
        entry.insert(QStringLiteral("total"), device.rows.size());
        entry.insert(QStringLiteral("passed"), counts.passed);
        entry.insert(QStringLiteral("failed"), counts.failed);
        entry.insert(QStringLiteral("skipped"), counts.skipped);
        entry.insert(QStringLiteral("invalid"), counts.invalid);
        entries.append(entry);
    }
    return entries;
}

/// @brief Counts devices only when every evaluated row has passed.
int ReportController::batchPassedDevices() const
{
    int passed = 0;
    for (const auto &device : m_devices) {
        const Counts counts = countOutcomes(device.rows);
        if (!device.rows.isEmpty() && counts.passed == device.rows.size()) ++passed;
    }
    return passed;
}

/// @brief Prefers the source CSV directory as the batch export location.
QUrl ReportController::suggestedBatchFolderUrl() const
{
    return m_sourceDirectory.isEmpty() ? QUrl{} : QUrl::fromLocalFile(m_sourceDirectory);
}

/// @brief Publishes a session error only when its text changes.
void ReportController::setError(const QString &message)
{
    if (m_errorMessage == message) return;
    m_errorMessage = message;
    emit errorMessageChanged();
}

/// @brief Builds a safe selected-device filename beside the imported CSV.
QUrl ReportController::suggestedPdfUrl() const
{
    if (m_reportId.isEmpty()) return {};
    const QString safeSerial = safeFilePart(m_serialNumber.isEmpty() ? QStringLiteral("NO-SERIAL") : m_serialNumber);
    const QString name = QStringLiteral("SLX_%1_%2.pdf").arg(safeSerial, m_reportId);
    return QUrl::fromLocalFile(QDir(m_sourceDirectory).filePath(name));
}

/// @brief Accepts a parsed CSV as a new session only after schema validation, then
/// isolates rows by serial.
bool ReportController::importFromDevice(QIODevice &device, const QString &sourceName,
                                        const QString &sourceDirectory, bool example)
{
    const QByteArray contents = device.readAll();
    const slx::ImportResult imported = slx::parseCsv(contents);
    if (!imported.error.isEmpty()) {
        setError(imported.error);
        return false;
    }
    // Preserve device order from the CSV and isolate rows by serial. Records
    // without a serial go to an UNASSIGNED invalid-data report.
    QVector<DeviceRun> nextDevices;
    QHash<QString, int> deviceIndex;
    for (const auto &row : imported.rows) {
        const QString key = row.serial.isEmpty() ? QString(QChar(1)) : row.serial;
        if (!deviceIndex.contains(key)) {
            deviceIndex.insert(key, nextDevices.size());
            DeviceRun run;
            run.serial = row.serial.isEmpty() ? QStringLiteral("UNASSIGNED") : row.serial;
            nextDevices.append(run);
        }
        nextDevices[deviceIndex.value(key)].rawRows.append(row);
    }
    const QDateTime importedAt = QDateTime::currentDateTime();
    if (m_exampleLoaded && !example) {
        m_operatorName.clear();
        m_stationId.clear();
    }
    if (m_exampleProfileLoaded && !example) {
        m_profile = {};
        m_profileActive = false;
        m_exampleProfileLoaded = false;
        emit profileChanged();
    }
    m_modelName = m_profileActive ? m_profile.model : QString{};
    m_procedureRevision = m_profileActive ? m_profile.revision : QString{};
    m_testDate = importedAt.date().toString(Qt::ISODate);
    m_exampleLoaded = example;
    m_sourceFile = sourceName;
    m_sourceDirectory = sourceDirectory;
    m_sourceHash = QString::fromLatin1(QCryptographicHash::hash(contents, QCryptographicHash::Sha256).toHex());
    m_baseReportId = QStringLiteral("TR-%1").arg(importedAt.toUTC().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz")));
    m_importedAt = importedAt.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss t"));
    for (int i = 0; i < nextDevices.size(); ++i) {
        nextDevices[i].reportId = QStringLiteral("%1-%2")
            .arg(m_baseReportId, QStringLiteral("%1").arg(i + 1, 3, 10, QLatin1Char('0')));
    }
    m_devices = std::move(nextDevices);
    m_selectedDeviceIndex = 0;
    refreshAllDevices();
    setError({});
    emit metadataChanged();
    return true;
}

/// @brief Reevaluates original rows for every device whenever the active profile
/// changes.
void ReportController::refreshAllDevices()
{
    for (auto &device : m_devices) {
        device.rows = m_profileActive
            ? slx::applyProfile(device.rawRows, m_profile, device.serial) : device.rawRows;
    }
    refreshSelectedDevice();
}

/// @brief Synchronizes the selected device's model, counts, identifiers, and analysis
/// highlights.
void ReportController::refreshSelectedDevice()
{
    if (m_selectedDeviceIndex < 0 || m_selectedDeviceIndex >= m_devices.size()) {
        m_serialNumber.clear();
        m_reportId.clear();
        m_results.setResults({});
        m_passedCount = m_failedCount = m_skippedCount = m_invalidCount = m_boundedCount = 0;
        m_toleranceHighlights.clear();
        emit dataChanged();
        return;
    }
    const auto &device = m_devices.at(m_selectedDeviceIndex);
    m_serialNumber = device.serial;
    m_reportId = device.reportId;
    const Counts counts = countOutcomes(device.rows);
    m_passedCount = counts.passed;
    m_failedCount = counts.failed;
    m_skippedCount = counts.skipped;
    m_invalidCount = counts.invalid;
    m_toleranceHighlights = buildToleranceHighlights(device.rows, m_boundedCount);
    m_results.setResults(device.rows);
    emit dataChanged();
}

/// @brief Changes the visible device only for a valid index, preserving all other runs.
bool ReportController::selectDevice(int index)
{
    if (index < 0 || index >= m_devices.size()) {
        setError(QStringLiteral("Select a valid device."));
        return false;
    }
    if (index == m_selectedDeviceIndex) return true;
    m_selectedDeviceIndex = index;
    refreshSelectedDevice();
    setError({});
    return true;
}

/// @brief Commits a fully parsed and hashed profile, then reevaluates existing
/// measurements.
bool ReportController::loadProfileBytes(const QByteArray &contents, const QString &sourceName, bool bundled)
{
    slx::ProfileResult parsed = slx::parseProfileJson(contents);
    if (!parsed.error.isEmpty()) {
        setError(parsed.error);
        return false;
    }
    parsed.profile.fileName = sourceName;
    parsed.profile.hash = QString::fromLatin1(QCryptographicHash::hash(contents, QCryptographicHash::Sha256).toHex());
    m_profile = std::move(parsed.profile);
    m_profileActive = true;
    m_exampleProfileLoaded = bundled;
    m_modelName = m_profile.model;
    m_procedureRevision = m_profile.revision;
    refreshAllDevices();
    setError({});
    emit profileChanged();
    emit metadataChanged();
    return true;
}

/// @brief Reads a local profile file and leaves the active plan unchanged if it fails
/// validation.
bool ReportController::loadProfile(const QUrl &url)
{
    if (!url.isLocalFile()) {
        setError(QStringLiteral("Select a local JSON test profile."));
        return false;
    }
    QFile file(url.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) {
        setError(QStringLiteral("Cannot open test profile: %1").arg(file.errorString()));
        return false;
    }
    return loadProfileBytes(file.readAll(), QFileInfo(file).fileName(), false);
}

/// @brief Restores raw CSV verdicts and clears identity fields supplied by the profile.
void ReportController::clearProfile()
{
    if (!m_profileActive) return;
    m_profile = {};
    m_profileActive = false;
    m_exampleProfileLoaded = false;
    m_modelName.clear();
    m_procedureRevision.clear();
    refreshAllDevices();
    setError({});
    emit profileChanged();
    emit metadataChanged();
}

/// @brief Reads a local CSV and retains the accepted session if opening or parsing
/// fails.
bool ReportController::importCsv(const QUrl &url)
{
    if (!url.isLocalFile()) {
        setError(QStringLiteral("Select a local CSV file."));
        return false;
    }
    QFile file(url.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) {
        setError(QStringLiteral("Cannot open CSV: %1").arg(file.errorString()));
        return false;
    }
    const QFileInfo source(file);
    return importFromDevice(file, source.fileName(), source.absolutePath(), false);
}

/// @brief Loads the bundled single-device CSV and its independent test profile.
bool ReportController::loadExample()
{
    return loadBundledExample(QStringLiteral("demo.csv"), QStringLiteral("demo-profile.json"));
}

/// @brief Loads the bundled multi-device CSV and its independent test profile.
bool ReportController::loadBatchExample()
{
    return loadBundledExample(QStringLiteral("batch.csv"), QStringLiteral("batch-profile.json"));
}

/// @brief Loads a named resource pair and supplies sample operator metadata for
/// evaluation.
bool ReportController::loadBundledExample(const QString &csvName, const QString &profileName)
{
    QFile file(QStringLiteral(":/examples/") + csvName);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(QStringLiteral("The bundled example is unavailable."));
        return false;
    }
    QString directory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (directory.isEmpty()) directory = QDir::homePath();
    if (!importFromDevice(file, csvName, directory, true)) return false;
    QFile profileFile(QStringLiteral(":/examples/") + profileName);
    if (!profileFile.open(QIODevice::ReadOnly)
        || !loadProfileBytes(profileFile.readAll(), profileName, true)) {
        setError(QStringLiteral("The bundled test profile is unavailable or invalid."));
        return false;
    }
    setOperatorName(QStringLiteral("Example operator"));
    setStationId(QStringLiteral("BENCH-01"));
    return true;
}

/// @brief Rejects export until a run and all required report identity fields are
/// present.
bool ReportController::validateExport()
{
    if (m_devices.isEmpty()) {
        setError(QStringLiteral("Import a CSV before exporting a report."));
        return false;
    }
    if (m_modelName.trimmed().isEmpty() || m_operatorName.trimmed().isEmpty()
        || m_stationId.trimmed().isEmpty() || m_procedureRevision.trimmed().isEmpty()) {
        setError(QStringLiteral("Enter model, operator, station and procedure revision."));
        return false;
    }
    if (!QDate::fromString(m_testDate, Qt::ISODate).isValid()) {
        setError(QStringLiteral("Enter a valid date in YYYY-MM-DD format."));
        return false;
    }
    return true;
}

/// @brief Snapshots one device's identity, provenance, and verdict for PDF generation.
slx::ReportMetadata ReportController::metadataForDevice(int index) const
{
    const auto &device = m_devices.at(index);
    const Counts counts = countOutcomes(device.rows);
    return {m_modelName.trimmed(), device.serial, m_testDate, m_operatorName.trimmed(),
            m_stationId.trimmed(), m_procedureRevision.trimmed(), device.reportId,
            m_importedAt, m_sourceFile, m_sourceHash,
            m_profileActive ? m_profile.fileName : QString{},
            m_profileActive ? m_profile.hash : QString{},
            verdictFor(counts, device.rows.size()),
            counts.passed, counts.failed, counts.skipped, counts.invalid};
}

/// @brief Writes the selected device's PDF and emits success only after the file is
/// committed.
bool ReportController::exportPdf(const QUrl &url)
{
    if (!validateExport()) return false;
    if (!url.isLocalFile()) {
        setError(QStringLiteral("Select a local PDF destination."));
        return false;
    }
    QString path = url.toLocalFile();
    if (!path.endsWith(QStringLiteral(".pdf"), Qt::CaseInsensitive)) path += QStringLiteral(".pdf");
    QString error;
    if (!slx::writePdf(path, metadataForDevice(m_selectedDeviceIndex),
                       m_devices.at(m_selectedDeviceIndex).rows, error)) {
        setError(error);
        return false;
    }
    setError({});
    emit exportSucceeded(path);
    return true;
}

/// @brief Stages every device PDF and publishes the complete batch with one directory
/// rename.
bool ReportController::exportAllPdfs(const QUrl &folder)
{
    if (!validateExport()) return false;
    if (!folder.isLocalFile()) {
        setError(QStringLiteral("Select a local output folder."));
        return false;
    }
    const QDir base(folder.toLocalFile());
    if (!base.exists()) {
        setError(QStringLiteral("The selected output folder does not exist."));
        return false;
    }
    QTemporaryDir staging(base.filePath(QStringLiteral(".slx-reports-XXXXXX")));
    if (!staging.isValid()) {
        setError(QStringLiteral("Cannot create a temporary report folder."));
        return false;
    }
    for (int i = 0; i < m_devices.size(); ++i) {
        const auto &device = m_devices.at(i);
        const QString fileName = QStringLiteral("SLX_%1_%2.pdf")
            .arg(safeFilePart(device.serial), device.reportId);
        QString error;
        if (!slx::writePdf(QDir(staging.path()).filePath(fileName),
                           metadataForDevice(i), device.rows, error)) {
            setError(QStringLiteral("%1: %2").arg(device.serial, error));
            return false;
        }
    }
    const QString stem = QStringLiteral("SLX_Reports_%1").arg(safeFilePart(m_baseReportId));
    QString destination = base.filePath(stem);
    for (int suffix = 2; QFileInfo::exists(destination); ++suffix)
        destination = base.filePath(QStringLiteral("%1_%2").arg(stem).arg(suffix));
    if (!QDir().rename(staging.path(), destination)) {
        setError(QStringLiteral("Cannot publish the completed report folder."));
        return false;
    }
    staging.setAutoRemove(false);
    setError({});
    emit batchExportSucceeded(destination, m_devices.size());
    return true;
}
