// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

/// @file
/// @brief Exposes report workflow state and actions to the QML interface.

#pragma once

#include "resultsmodel.h"
#include "profiledata.h"

#include <QDate>
#include <QIODevice>
#include <QObject>
#include <QUrl>
#include <QVariantList>

namespace slx { struct ReportMetadata; }

/// Owns imported measurements and exposes one selected device to the QML views.
/// Failed imports and profile replacements leave the accepted run available.
class ReportController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString modelName READ modelName WRITE setModelName NOTIFY metadataChanged)
    Q_PROPERTY(QString operatorName READ operatorName WRITE setOperatorName NOTIFY metadataChanged)
    Q_PROPERTY(QString stationId READ stationId WRITE setStationId NOTIFY metadataChanged)
    Q_PROPERTY(QString procedureRevision READ procedureRevision WRITE setProcedureRevision NOTIFY metadataChanged)
    Q_PROPERTY(QString testDate READ testDate WRITE setTestDate NOTIFY metadataChanged)
    Q_PROPERTY(QString serialNumber READ serialNumber NOTIFY dataChanged)
    Q_PROPERTY(QString sourceFile READ sourceFile NOTIFY dataChanged)
    Q_PROPERTY(QString sourceHash READ sourceHash NOTIFY dataChanged)
    Q_PROPERTY(QString reportId READ reportId NOTIFY dataChanged)
    Q_PROPERTY(QString importedAt READ importedAt NOTIFY dataChanged)
    Q_PROPERTY(QUrl suggestedPdfUrl READ suggestedPdfUrl NOTIFY dataChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(int totalCount READ totalCount NOTIFY dataChanged)
    Q_PROPERTY(int passedCount READ passedCount NOTIFY dataChanged)
    Q_PROPERTY(int failedCount READ failedCount NOTIFY dataChanged)
    Q_PROPERTY(int skippedCount READ skippedCount NOTIFY dataChanged)
    Q_PROPERTY(int invalidCount READ invalidCount NOTIFY dataChanged)
    Q_PROPERTY(int boundedCount READ boundedCount NOTIFY dataChanged)
    Q_PROPERTY(QVariantList toleranceHighlights READ toleranceHighlights NOTIFY dataChanged)
    Q_PROPERTY(QVariantList previewRows READ previewRows NOTIFY dataChanged)
    Q_PROPERTY(QString overallOutcome READ overallOutcome NOTIFY dataChanged)
    Q_PROPERTY(int deviceCount READ deviceCount NOTIFY dataChanged)
    Q_PROPERTY(int selectedDeviceIndex READ selectedDeviceIndex NOTIFY dataChanged)
    Q_PROPERTY(QVariantList devices READ devices NOTIFY dataChanged)
    Q_PROPERTY(int batchPassedDevices READ batchPassedDevices NOTIFY dataChanged)
    Q_PROPERTY(int batchAttentionDevices READ batchAttentionDevices NOTIFY dataChanged)
    Q_PROPERTY(QUrl suggestedBatchFolderUrl READ suggestedBatchFolderUrl NOTIFY dataChanged)
    Q_PROPERTY(bool profileActive READ profileActive NOTIFY profileChanged)
    Q_PROPERTY(QString profileFile READ profileFile NOTIFY profileChanged)
    Q_PROPERTY(QString profileHash READ profileHash NOTIFY profileChanged)
    Q_PROPERTY(QString profileRevision READ profileRevision NOTIFY profileChanged)

public:
    /// @brief Starts a report session with an empty device set and today's test date.
    explicit ReportController(QObject *parent = nullptr);

    /// @brief Complete result model for the device currently shown in QML.
    ResultsModel *resultsModel() { return &m_results; }
    /// @brief Device model; an active profile supplies and locks this field.
    QString modelName() const { return m_modelName; }
    /// @brief Operator recorded in the report header.
    QString operatorName() const { return m_operatorName; }
    /// @brief Bench or station recorded in the report header.
    QString stationId() const { return m_stationId; }
    /// @brief Procedure revision; an active profile supplies and locks this field.
    QString procedureRevision() const { return m_procedureRevision; }
    /// @brief Test date as entered; PDF export requires a valid ISO date.
    QString testDate() const { return m_testDate; }
    /// @brief Serial associated with the selected device's CSV rows.
    QString serialNumber() const { return m_serialNumber; }
    /// @brief CSV filename used in the report; the local directory is omitted.
    QString sourceFile() const { return m_sourceFile; }
    /// @brief SHA-256 digest of the CSV bytes, computed before parsing.
    QString sourceHash() const { return m_sourceHash; }
    /// @brief Report identifier assigned at import and retained during inspection.
    QString reportId() const { return m_reportId; }
    /// @brief Local timestamp recorded when the CSV was accepted.
    QString importedAt() const { return m_importedAt; }
    /// @brief Proposes a selected-device PDF path beside the imported CSV.
    QUrl suggestedPdfUrl() const;
    /// @brief Last import or export error; empty after a successful operation.
    QString errorMessage() const { return m_errorMessage; }
    /// @brief Evaluated row count, including required tests missing from the CSV.
    int totalCount() const { return m_results.rowCount(); }
    /// @brief Passed numeric measurements and functional checks.
    int passedCount() const { return m_passedCount; }
    /// @brief Out-of-tolerance measurements and failed functional checks.
    int failedCount() const { return m_failedCount; }
    /// @brief Checks declared skipped or not run by the bench.
    int skippedCount() const { return m_skippedCount; }
    /// @brief Rows with malformed, conflicting, or missing evidence.
    int invalidCount() const { return m_invalidCount; }
    /// @brief Numeric results with a usable lower and upper limit.
    int boundedCount() const { return m_boundedCount; }
    /// @brief Up to six readings nearest to or outside their own limits.
    QVariantList toleranceHighlights() const { return m_toleranceHighlights; }
    /// @brief First twelve results for the preview; PDF export uses the full model.
    QVariantList previewRows() const;
    /// @brief Combined verdict; failed, invalid, and skipped states remain explicit.
    QString overallOutcome() const;
    /// @brief Device groups in the CSV, including the unassigned group if present.
    int deviceCount() const { return m_devices.size(); }
    /// @brief Selected device index, or -1 before any import.
    int selectedDeviceIndex() const { return m_selectedDeviceIndex; }
    /// @brief Device selector summaries in order of first appearance in the CSV.
    QVariantList devices() const;
    /// @brief Devices for which every evaluated row passed.
    int batchPassedDevices() const;
    /// @brief Devices with failed, invalid, or incomplete tests.
    int batchAttentionDevices() const { return deviceCount() - batchPassedDevices(); }
    /// @brief Proposes the source CSV directory for batch publication.
    QUrl suggestedBatchFolderUrl() const;
    /// @brief True when a loaded profile supplies the acceptance criteria.
    bool profileActive() const { return m_profileActive; }
    /// @brief Active profile filename; empty without a loaded profile.
    QString profileFile() const { return m_profileActive ? m_profile.fileName : QString{}; }
    /// @brief Digest of the original profile bytes; empty without a loaded profile.
    QString profileHash() const { return m_profileActive ? m_profile.hash : QString{}; }
    /// @brief Revision recorded in the active profile; empty without one.
    QString profileRevision() const { return m_profileActive ? m_profile.revision : QString{}; }

    /// @brief Accepts manual model edits only when no active profile fixes the value.
    void setModelName(const QString &value);
    /// @brief Updates the operator identity and emits metadataChanged on change.
    void setOperatorName(const QString &value);
    /// @brief Updates the station identity and emits metadataChanged on change.
    void setStationId(const QString &value);
    /// @brief Accepts manual revision edits only when no active profile fixes the
    /// value.
    void setProcedureRevision(const QString &value);
    /// @brief Updates the report date; export validation rejects invalid ISO dates.
    void setTestDate(const QString &value);

    /// @brief Accepts a local CSV only after parsing succeeds; earlier report state
    /// survives failure.
    /// @return False with errorMessage set when the source cannot be accepted.
    Q_INVOKABLE bool importCsv(const QUrl &url);
    /// @brief Loads the bundled single-device measurement and profile pair.
    Q_INVOKABLE bool loadExample();
    /// @brief Loads the bundled multi-device measurement and profile pair.
    Q_INVOKABLE bool loadBatchExample();
    /// @brief Loads a local plan without replacing an accepted plan on failure.
    /// @return False with errorMessage set when the plan is invalid or unreadable.
    Q_INVOKABLE bool loadProfile(const QUrl &url);
    /// @brief Removes plan limits and reevaluates the original CSV evidence.
    Q_INVOKABLE void clearProfile();
    /// @brief Changes the inspected device for a valid index without altering other
    /// runs.
    Q_INVOKABLE bool selectDevice(int index);
    /// @brief Commits the selected device's PDF before signaling success.
    /// @return False with errorMessage set when validation or writing fails.
    Q_INVOKABLE bool exportPdf(const QUrl &url);
    /// @brief Publishes all device PDFs as one completed folder.
    /// @return False without publishing a partial batch when any device fails.
    Q_INVOKABLE bool exportAllPdfs(const QUrl &folder);

signals:
    /// @brief Notifies the view that editable report identity changed.
    void metadataChanged();
    /// @brief Notifies the view that selected-device evidence or analysis changed.
    void dataChanged();
    /// @brief Notifies the view that the workflow diagnostic changed.
    void errorMessageChanged();
    /// @brief Reports the path of a committed selected-device PDF.
    void exportSucceeded(const QString &path);
    /// @brief Reports the published folder and number of device PDFs.
    void batchExportSucceeded(const QString &folder, int count);
    /// @brief Notifies the view that independent criteria changed.
    void profileChanged();

private:
    /// @brief Parses and commits a new measurement source without replacing
    /// accepted results on failure.
    bool importFromDevice(QIODevice &device, const QString &sourceName,
                          const QString &sourceDirectory, bool example);
    /// @brief Reads the named resource pair and sets sample metadata.
    bool loadBundledExample(const QString &csvName, const QString &profileName);
    /// @brief Validates and hashes a plan before replacing the active criteria.
    bool loadProfileBytes(const QByteArray &contents, const QString &sourceName, bool bundled);
    /// @brief Reevaluates every device from untouched source measurements.
    void refreshAllDevices();
    /// @brief Refreshes visible rows, counters, and highlights for the selected serial.
    void refreshSelectedDevice();
    /// @brief Checks required report identity and date before writing any PDF.
    bool validateExport();
    /// @brief Builds one immutable export snapshot from selected session metadata.
    slx::ReportMetadata metadataForDevice(int index) const;
    /// @brief Stores and signals a changed workflow diagnostic.
    void setError(const QString &message);

    struct DeviceRun {
        QString serial;
        QString reportId;
        QVector<slx::TestResult> rawRows;
        QVector<slx::TestResult> rows;
    };

    ResultsModel m_results;
    QVector<DeviceRun> m_devices;
    int m_selectedDeviceIndex = -1;
    slx::TestProfile m_profile;
    bool m_profileActive = false;
    bool m_exampleProfileLoaded = false;
    QString m_modelName;
    QString m_operatorName;
    QString m_stationId;
    QString m_procedureRevision;
    QString m_testDate;
    QString m_serialNumber;
    QString m_sourceFile;
    QString m_sourceDirectory;
    QString m_sourceHash;
    QString m_reportId;
    QString m_baseReportId;
    QString m_importedAt;
    QString m_errorMessage;
    bool m_exampleLoaded = false;
    int m_passedCount = 0;
    int m_failedCount = 0;
    int m_skippedCount = 0;
    int m_invalidCount = 0;
    int m_boundedCount = 0;
    QVariantList m_toleranceHighlights;
};
