// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

/// @file
/// @brief Implements lossless CSV draft editing and atomic persistence.

#include "csvcomposer.h"

#include "reportdata.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStringList>

#include <utility>

/// @brief Starts a draft with one editable row and a default Documents destination.
CsvComposer::CsvComposer(QObject *parent) : QAbstractListModel(parent)
{
    m_entries.append(Entry{});
    const QString documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    m_suggestedUrl = QUrl::fromLocalFile(QDir(documents.isEmpty() ? QDir::homePath() : documents)
                                               .filePath(QStringLiteral("SLX_test_results.csv")));
}

/// @brief Exposes top-level draft rows; child indexes contain no entries.
int CsvComposer::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_entries.size());
}

/// @brief Returns the requested cell text, or an empty value for an invalid index or
/// role.
QVariant CsvComposer::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size()) return {};
    const Entry &entry = m_entries.at(index.row());
    switch (role) {
    case SerialRole: return entry.serial;
    case TestRole: return entry.test;
    case ValueRole: return entry.value;
    case UnitRole: return entry.unit;
    case MinimumRole: return entry.minimum;
    case MaximumRole: return entry.maximum;
    case StatusRole: return entry.status;
    case DiagnosticRole: return entry.diagnostic;
    default: return {};
    }
}

/// @brief Defines the stable cell names consumed by the QML editor.
QHash<int, QByteArray> CsvComposer::roleNames() const
{
    return {{SerialRole, "serial"}, {TestRole, "test"}, {ValueRole, "value"},
            {UnitRole, "unit"}, {MinimumRole, "minimum"}, {MaximumRole, "maximum"},
            {StatusRole, "status"}, {DiagnosticRole, "diagnostic"}};
}

/// @brief Notifies observers only when the editor error actually changes.
void CsvComposer::setError(const QString &message)
{
    if (m_error == message) return;
    m_error = message;
    emit errorChanged();
}

/// @brief Resets editor state without changing the report already imported by the
/// controller.
void CsvComposer::newDraft()
{
    beginResetModel();
    m_entries = {Entry{}};
    endResetModel();
    m_draftName = QStringLiteral("New CSV");
    m_dirty = false;
    m_savedUrl = QUrl{};
    const QString documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    m_suggestedUrl = QUrl::fromLocalFile(QDir(documents.isEmpty() ? QDir::homePath() : documents)
                                               .filePath(QStringLiteral("SLX_test_results.csv")));
    setError({});
    emit draftChanged();
    emit savedLocationChanged();
}

/// @brief Appends an empty test and carries the previous serial into batch entry.
void CsvComposer::addEntry()
{
    const int next = m_entries.size();
    beginInsertRows({}, next, next);
    Entry entry;
    if (!m_entries.isEmpty()) {
        entry.serial = m_entries.constLast().serial;
        entry.inheritedSerial = !entry.serial.isEmpty();
    }
    m_entries.append(entry);
    endInsertRows();
    m_dirty = true;
    setError({});
    emit draftChanged();
}

/// @brief Inserts a copy after a valid row; ignores indexes outside the draft.
void CsvComposer::duplicateEntry(int index)
{
    if (index < 0 || index >= m_entries.size()) return;
    beginInsertRows({}, index + 1, index + 1);
    m_entries.insert(index + 1, m_entries.at(index));
    endInsertRows();
    m_dirty = true;
    setError({});
    emit draftChanged();
}

/// @brief Removes a row but keeps one editable placeholder when the last row is
/// cleared.
void CsvComposer::removeEntry(int index)
{
    if (index < 0 || index >= m_entries.size()) return;
    if (m_entries.size() == 1) {
        m_entries[0] = {};
        emit dataChanged(this->index(0), this->index(0));
    } else {
        beginRemoveRows({}, index, index);
        m_entries.removeAt(index);
        endRemoveRows();
    }
    m_dirty = true;
    setError({});
    emit draftChanged();
}

/// @brief Updates a recognized cell and marks the draft dirty only when its value
/// changes.
void CsvComposer::setCell(int index, const QString &column, const QString &value)
{
    if (index < 0 || index >= m_entries.size()) return;
    Entry &entry = m_entries[index];
    QString *cell = nullptr;
    int role = 0;
    if (column == QStringLiteral("serial")) { cell = &entry.serial; role = SerialRole; }
    else if (column == QStringLiteral("test")) { cell = &entry.test; role = TestRole; }
    else if (column == QStringLiteral("value")) { cell = &entry.value; role = ValueRole; }
    else if (column == QStringLiteral("unit")) { cell = &entry.unit; role = UnitRole; }
    else if (column == QStringLiteral("minimum")) { cell = &entry.minimum; role = MinimumRole; }
    else if (column == QStringLiteral("maximum")) { cell = &entry.maximum; role = MaximumRole; }
    else if (column == QStringLiteral("status")) { cell = &entry.status; role = StatusRole; }
    else if (column == QStringLiteral("diagnostic")) { cell = &entry.diagnostic; role = DiagnosticRole; }
    if (!cell || *cell == value) return;
    *cell = value;
    entry.inheritedSerial = false;
    emit dataChanged(this->index(index), this->index(index), {role});
    m_dirty = true;
    setError({});
    emit draftChanged();
}

/// @brief Replaces the draft only if a local CSV parses without columns the editor
/// would discard.
bool CsvComposer::loadCsv(const QUrl &url)
{
    if (!url.isLocalFile()) {
        setError(QStringLiteral("Choose a local CSV file."));
        return false;
    }
    QFile file(url.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) {
        setError(QStringLiteral("Cannot open CSV: %1").arg(file.errorString()));
        return false;
    }
    const slx::ImportResult parsed = slx::parseCsv(file.readAll());
    if (!parsed.error.isEmpty()) {
        setError(parsed.error);
        return false;
    }
    if (!parsed.ignoredColumns.isEmpty()) {
        setError(QStringLiteral("The editor cannot preserve these CSV columns: %1.")
                     .arg(parsed.ignoredColumns.join(QStringLiteral(", "))));
        return false;
    }
    QVector<Entry> replacement;
    replacement.reserve(parsed.rows.size());
    for (const auto &row : parsed.rows) {
        replacement.append({row.serial, row.testName, row.measurement, row.unit,
                            row.minimum, row.maximum, row.reportedStatus, row.diagnostic});
    }
    beginResetModel();
    m_entries = std::move(replacement);
    endResetModel();
    m_draftName = QFileInfo(file).fileName();
    m_suggestedUrl = url;
    m_savedUrl = QUrl{};
    m_dirty = false;
    setError({});
    emit draftChanged();
    emit savedLocationChanged();
    return true;
}

/// @brief Recognizes an untouched placeholder even when its serial was inherited.
bool CsvComposer::empty(const Entry &entry)
{
    return (entry.serial.trimmed().isEmpty() || entry.inheritedSerial)
        && entry.test.trimmed().isEmpty()
        && entry.value.trimmed().isEmpty() && entry.unit.trimmed().isEmpty()
        && entry.minimum.trimmed().isEmpty() && entry.maximum.trimmed().isEmpty()
        && entry.status.trimmed().isEmpty() && entry.diagnostic.trimmed().isEmpty();
}

/// @brief Escapes separators, quotes, and line breaks for the semicolon CSV format.
QString CsvComposer::quoted(const QString &value)
{
    QString escaped = value;
    escaped.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    if (escaped.contains(QLatin1Char(';')) || escaped.contains(QLatin1Char('"'))
        || escaped.contains(QLatin1Char('\n')) || escaped.contains(QLatin1Char('\r'))) {
        return QLatin1Char('"') + escaped + QLatin1Char('"');
    }
    return escaped;
}

/// @brief Returns a row-scoped diagnostic for invalid identity, numbers, status, or
/// limits; an empty result means the row is valid.
QString CsvComposer::validate(const Entry &entry, int line)
{
    const QString prefix = QStringLiteral("Row %1: ").arg(line);
    if (entry.serial.trimmed().isEmpty()) return prefix + QStringLiteral("enter a serial number.");
    if (entry.test.trimmed().isEmpty()) return prefix + QStringLiteral("enter a test name.");
    if (entry.value.trimmed().isEmpty() && entry.status.trimmed().isEmpty())
        return prefix + QStringLiteral("enter a measurement or status.");
    const QPair<QString, QString> numericFields[] = {
        {QStringLiteral("measurement"), entry.value},
        {QStringLiteral("minimum"), entry.minimum},
        {QStringLiteral("maximum"), entry.maximum}
    };
    for (const auto &field : numericFields) {
        if (field.second.trimmed().isEmpty()) continue;
        double number = 0.0;
        if (!slx::parseNumber(field.second, number))
            return prefix + QStringLiteral("%1 must be a finite number.").arg(field.first);
    }
    if (slx::parseStatus(entry.status) == slx::DeclaredStatus::Unknown)
        return prefix + QStringLiteral("use PASS, FAIL, SKIPPED or ERROR for status.");
    if (!entry.minimum.trimmed().isEmpty() && !entry.maximum.trimmed().isEmpty()) {
        double lower = 0.0, upper = 0.0;
        slx::parseNumber(entry.minimum, lower);
        slx::parseNumber(entry.maximum, upper);
        if (lower > upper) return prefix + QStringLiteral("minimum exceeds maximum.");
    }
    return {};
}

/// @brief Validates nonempty rows and commits UTF-8 CSV through QSaveFile; failed
/// validation leaves the target untouched.
bool CsvComposer::saveCsv(const QUrl &url)
{
    if (!url.isLocalFile()) {
        setError(QStringLiteral("Choose a local CSV destination."));
        return false;
    }
    QString path = url.toLocalFile();
    if (!path.endsWith(QStringLiteral(".csv"), Qt::CaseInsensitive)) path += QStringLiteral(".csv");
    QByteArray bytes("serial;test;value;unit;min;max;status;diagnostic\r\n");
    int writtenRows = 0;
    for (int i = 0; i < m_entries.size(); ++i) {
        const Entry &entry = m_entries.at(i);
        if (empty(entry)) continue;
        const QString problem = validate(entry, i + 1);
        if (!problem.isEmpty()) {
            setError(problem);
            return false;
        }
        const QStringList cells = {entry.serial, entry.test, entry.value, entry.unit,
                                   entry.minimum, entry.maximum, entry.status, entry.diagnostic};
        QStringList escaped;
        for (const auto &cell : cells) escaped.append(quoted(cell.trimmed()));
        bytes += escaped.join(QLatin1Char(';')).toUtf8();
        bytes += "\r\n";
        ++writtenRows;
    }
    if (writtenRows == 0) {
        setError(QStringLiteral("Add at least one test before saving."));
        return false;
    }
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        setError(QStringLiteral("Cannot create CSV: %1").arg(file.errorString()));
        return false;
    }
    if (file.write(bytes) != bytes.size() || !file.commit()) {
        setError(QStringLiteral("Cannot complete CSV: %1").arg(file.errorString()));
        return false;
    }
    m_savedUrl = QUrl::fromLocalFile(path);
    m_suggestedUrl = m_savedUrl;
    m_draftName = QFileInfo(path).fileName();
    m_dirty = false;
    setError({});
    emit draftChanged();
    emit savedLocationChanged();
    return true;
}
