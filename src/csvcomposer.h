// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

/// @file
/// @brief Declares the editor model for preparing bench CSV input.

#pragma once

#include <QAbstractListModel>
#include <QUrl>
#include <QVector>

/// Holds an editable CSV draft independently of the imported report.
class CsvComposer final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int entryCount READ entryCount NOTIFY draftChanged)
    Q_PROPERTY(bool dirty READ dirty NOTIFY draftChanged)
    Q_PROPERTY(QString draftName READ draftName NOTIFY draftChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorChanged)
    Q_PROPERTY(QUrl savedUrl READ savedUrl NOTIFY savedLocationChanged)
    Q_PROPERTY(QUrl suggestedUrl READ suggestedUrl NOTIFY draftChanged)
    Q_PROPERTY(QString savedPath READ savedPath NOTIFY savedLocationChanged)

public:
    /// @brief Creates an editable draft with one row and a suggested Documents path.
    explicit CsvComposer(QObject *parent = nullptr);

    enum Role {
        SerialRole = Qt::UserRole + 1,
        TestRole,
        ValueRole,
        UnitRole,
        MinimumRole,
        MaximumRole,
        StatusRole,
        DiagnosticRole
    };

    /// @brief Reports editable rows only for a top-level model index.
    int rowCount(const QModelIndex &parent = {}) const override;
    /// @brief Returns source cell text for a named editor role.
    QVariant data(const QModelIndex &index, int role) const override;
    /// @brief Provides the role names bound by QML input cells.
    QHash<int, QByteArray> roleNames() const override;

    /// @brief Draft row count, including the empty row available for input.
    int entryCount() const { return static_cast<int>(m_entries.size()); }
    /// @brief True when the draft contains edits made since reset, load, or save.
    bool dirty() const { return m_dirty; }
    /// @brief Filename shown in the editor, or "New CSV" for an unsaved sheet.
    QString draftName() const { return m_draftName; }
    /// @brief Last open or save error; empty after a successful operation.
    QString errorMessage() const { return m_error; }
    /// @brief Destination of the last successful save, if any.
    QUrl savedUrl() const { return m_savedUrl; }
    /// @brief Initial destination shown by the save dialog.
    QUrl suggestedUrl() const { return m_suggestedUrl; }
    /// @brief Local path passed to the report importer after a successful save.
    QString savedPath() const { return m_savedUrl.toLocalFile(); }

    /// @brief Discards editor edits and starts one empty row without changing report
    /// evidence.
    Q_INVOKABLE void newDraft();
    /// @brief Appends a row with the preceding serial prefilled for batch entry.
    Q_INVOKABLE void addEntry();
    /// @brief Copies a valid row directly below its source; ignores invalid indexes.
    Q_INVOKABLE void duplicateEntry(int index);
    /// @brief Deletes a valid row while retaining one placeholder when the sheet is
    /// empty.
    Q_INVOKABLE void removeEntry(int index);
    /// @brief Updates a recognized column and marks the draft dirty only for a real
    /// change.
    Q_INVOKABLE void setCell(int index, const QString &column, const QString &value);
    /// @brief Replaces the draft only when a local CSV can be edited without losing
    /// columns.
    /// @return False with errorMessage set when opening or parsing fails.
    Q_INVOKABLE bool loadCsv(const QUrl &url);
    /// @brief Validates and atomically saves a semicolon-delimited UTF-8 CSV.
    /// @return False with errorMessage set; an existing target is left intact.
    Q_INVOKABLE bool saveCsv(const QUrl &url);

signals:
    /// @brief Notifies QML that row data, name, or dirty state changed.
    void draftChanged();
    /// @brief Notifies QML that the editor diagnostic changed.
    void errorChanged();
    /// @brief Notifies QML that the committed CSV destination changed.
    void savedLocationChanged();

private:
    struct Entry {
        QString serial;
        QString test;
        QString value;
        QString unit;
        QString minimum;
        QString maximum;
        QString status;
        QString diagnostic;
        bool inheritedSerial = false;
    };

    /// @brief Identifies an unused row, including a carried serial with no test data.
    static bool empty(const Entry &entry);
    /// @brief Escapes a cell for the semicolon CSV dialect.
    static QString quoted(const QString &value);
    /// @brief Returns a row-specific diagnostic or an empty string for an accepted row.
    static QString validate(const Entry &entry, int line);
    /// @brief Stores and signals a changed editor diagnostic.
    void setError(const QString &message);

    QVector<Entry> m_entries;
    QString m_draftName = QStringLiteral("New CSV");
    QString m_error;
    QUrl m_savedUrl;
    QUrl m_suggestedUrl;
    bool m_dirty = false;
};
