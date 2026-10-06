// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

/// @file
/// @brief Renders verified device evidence as a paginated A4 PDF.

#include "pdfexport.h"

#include <QColor>
#include <QFont>
#include <QFontMetricsF>
#include <QLinearGradient>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QSaveFile>
#include <QStringList>
#include <QSvgRenderer>

namespace slx {
namespace {

// Use 72 DPI so the layout coordinates match PDF points. Rows with
// diagnostics grow vertically; long cells are elided within fixed columns.
constexpr int left = 38;
constexpr int right = 557;
constexpr int bottom = 785;

const QColor ink("#18203B");
const QColor muted("#69738C");
const QColor border("#DFE4F0");
const QColor success("#167565");
const QColor red("#B4484E");
const QColor amber("#9B691B");
const QColor skipped("#687695");

/// @brief Applies the report's point size and weight to the active painter.
void setFont(QPainter &painter, int size, bool bold = false)
{
    QFont font(QStringLiteral("Arial"));
    font.setPointSize(size);
    font.setBold(bold);
    painter.setFont(font);
}

/// @brief Draws a table divider within the fixed printable column span.
void line(QPainter &painter, int y)
{
    painter.setPen(border);
    painter.drawLine(left, y, right, y);
}

/// @brief Fits a cell value to its measured width without changing the source text.
QString elide(QPainter &painter, const QString &text, int width)
{
    return QFontMetricsF(painter.font()).elidedText(text, Qt::ElideRight, width);
}

/// @brief Renders a bounded text cell using the report's common clipping and alignment
/// rules.
void drawText(QPainter &painter, const QRect &rect, const QString &value,
              const QColor &color = ink, Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignVCenter)
{
    painter.setPen(color);
    painter.drawText(rect, alignment, elide(painter, value, rect.width()));
}

/// @brief Prints the vector wordmark and page identity on the dark report masthead.
void drawHeader(QPainter &painter, int page)
{
    // The light wordmark needs a dark backing in print. Render the SVG
    // directly to preserve sharp edges at any PDF scale.
    painter.fillRect(QRect(left, 5, right - left, 50), QColor("#11172F"));
    QSvgRenderer logo(QStringLiteral(":/brand/logo.svg"));
    logo.render(&painter, QRectF(left + 8, 8, 88, 44));
    setFont(painter, 8);
    drawText(painter, QRect(360, 19, 187, 20),
             QStringLiteral("SLX TEST REPORT  ·  %1").arg(page), QColor("#F2F2F2"),
             Qt::AlignRight | Qt::AlignVCenter);
    QLinearGradient trace(left, 0, right, 0);
    trace.setColorAt(0, QColor("#2948FF"));
    trace.setColorAt(1, QColor("#D100FF"));
    painter.fillRect(QRect(left, 57, right - left, 2), trace);
}

/// @brief Prints report identity and page number below the printable content area.
void drawFooter(QPainter &painter, int page, const ReportMetadata &meta)
{
    line(painter, 796);
    setFont(painter, 8);
    drawText(painter, QRect(left, 802, 400, 20), meta.reportId, muted);
    drawText(painter, QRect(490, 802, 67, 20), QStringLiteral("Page %1").arg(page), muted,
             Qt::AlignRight | Qt::AlignVCenter);
}

/// @brief Stacks a metadata label and value inside one fixed-width summary column.
void drawLabelValue(QPainter &painter, int x, int y, int width,
                    const QString &label, const QString &value)
{
    setFont(painter, 8, true);
    drawText(painter, QRect(x, y, width, 14), label.toUpper(), muted);
    setFont(painter, 10);
    drawText(painter, QRect(x, y + 17, width, 19), value);
}

/// @brief Prints device identity and outcome totals, then returns the vertical start of
/// the results table.
int drawFirstPageSummary(QPainter &painter, const ReportMetadata &meta, int total)
{
    setFont(painter, 22, true);
    drawText(painter, QRect(left, 78, 519, 35), QStringLiteral("Device test report"));
    setFont(painter, 9);
    drawText(painter, QRect(left, 119, 519, 20),
             QStringLiteral("Measurements, limits and declared outcomes"), muted);

    painter.fillRect(QRect(left, 151, 519, 132), QColor("#F3F5FC"));
    drawLabelValue(painter, 50, 161, 237, QStringLiteral("Model"), meta.model);
    drawLabelValue(painter, 303, 161, 240, QStringLiteral("Serial number"), meta.serial);
    drawLabelValue(painter, 50, 203, 237, QStringLiteral("Test date"), meta.date);
    drawLabelValue(painter, 303, 203, 240, QStringLiteral("Operator"), meta.operatorName);
    drawLabelValue(painter, 50, 245, 237, QStringLiteral("Station"), meta.stationId);
    drawLabelValue(painter, 303, 245, 240, QStringLiteral("Procedure revision"), meta.procedureRevision);

    const QColor outcomeColor = meta.failed > 0 ? red : meta.invalid > 0 ? amber
                              : meta.skipped > 0 ? skipped : success;
    setFont(painter, 8, true);
    drawText(painter, QRect(left, 291, 135, 16), QStringLiteral("OVERALL OUTCOME"), muted);
    setFont(painter, 11, true);
    drawText(painter, QRect(left, 309, 519, 24), meta.overallOutcome, outcomeColor);
    setFont(painter, 9);
    drawText(painter, QRect(left, 338, 519, 18),
             QStringLiteral("%1 tests  ·  %2 passed  ·  %3 failed  ·  %4 skipped  ·  %5 invalid")
                 .arg(total).arg(meta.passed).arg(meta.failed).arg(meta.skipped).arg(meta.invalid), muted);
    // Calculate segments from all imported rows, including incomplete ones;
    // the count labels remain visible when a segment is too narrow for text.
    const int counts[] = {meta.passed, meta.failed, meta.skipped, meta.invalid};
    const QColor colors[] = {success, red, skipped, amber};
    int barX = left;
    for (int i = 0; i < 4; ++i) {
        const int nextX = i == 3 ? right : barX + qRound(519.0 * counts[i] / total);
        painter.fillRect(QRect(barX, 370, nextX - barX, 11), colors[i]);
        barX = nextX;
    }
    setFont(painter, 7);
    drawText(painter, QRect(left, 393, 340, 15), QStringLiteral("Report ID: %1").arg(meta.reportId), muted);
    drawText(painter, QRect(383, 393, 174, 15), meta.importedAt, muted,
             Qt::AlignRight | Qt::AlignVCenter);
    drawText(painter, QRect(left, 410, 519, 15), QStringLiteral("CSV: %1").arg(meta.sourceFile), muted);
    drawText(painter, QRect(left, 427, 519, 15), QStringLiteral("SHA-256: %1").arg(meta.sourceHash), muted);
    if (!meta.profileFile.isEmpty()) {
        drawText(painter, QRect(left, 444, 519, 15),
                 QStringLiteral("Test profile: %1  ·  revision %2")
                     .arg(meta.profileFile, meta.procedureRevision), muted);
        drawText(painter, QRect(left, 461, 519, 15),
                 QStringLiteral("Profile SHA-256: %1").arg(meta.profileHash), muted);
        return 482;
    }
    return 450;
}

/// @brief Prints the fixed result-column labels at a page break or table start.
void drawTableHeader(QPainter &painter, int y)
{
    painter.fillRect(QRect(left, y, 519, 27), QColor("#EDEFFA"));
    setFont(painter, 8, true);
    drawText(painter, QRect(46, y + 3, 136, 20), QStringLiteral("TEST"), muted);
    drawText(painter, QRect(190, y + 3, 73, 20), QStringLiteral("VALUE"), muted);
    drawText(painter, QRect(270, y + 3, 36, 20), QStringLiteral("UNIT"), muted);
    drawText(painter, QRect(314, y + 3, 102, 20), QStringLiteral("MIN / MAX"), muted);
    drawText(painter, QRect(424, y + 3, 125, 20), QStringLiteral("OUTCOME"), muted);
}

/// @brief Allocates a second text line only when a result carries a diagnostic.
int heightForRow(const TestResult &row)
{
    return row.detail.isEmpty() ? 28 : 38;
}

/// @brief Formats the original CSV line reference for a result's diagnostic.
QString rowOrigin(const TestResult &row)
{
    return row.line > 0 ? QStringLiteral("CSV line %1").arg(row.line)
                        : QStringLiteral("Required by profile");
}

/// @brief Prints a result inside fixed columns and adds diagnostic detail for anomalous
/// rows.
void drawRow(QPainter &painter, int y, int index, const TestResult &row, int height)
{
    if (index % 2 == 1) painter.fillRect(QRect(left, y, 519, height), QColor("#F8F9FD"));
    const QColor statusColor = row.outcome == Outcome::Passed ? success
                             : row.outcome == Outcome::Failed ? red
                             : row.outcome == Outcome::Skipped ? skipped : amber;
    setFont(painter, 9, true);
    drawText(painter, QRect(46, y + 2, 136, 17),
             row.testName.isEmpty() ? QStringLiteral("Unnamed test") : row.testName);
    setFont(painter, 9);
    drawText(painter, QRect(190, y + 2, 73, 17), row.measurement.isEmpty() ? QStringLiteral("—")
                                                                            : displayNumber(row.measurement));
    drawText(painter, QRect(270, y + 2, 36, 17), row.unit.isEmpty() ? QStringLiteral("—") : row.unit);
    drawText(painter, QRect(314, y + 2, 102, 17),
             (row.minimum.isEmpty() ? QStringLiteral("—") : displayNumber(row.minimum))
                 + QStringLiteral(" / ")
                 + (row.maximum.isEmpty() ? QStringLiteral("—") : displayNumber(row.maximum)));
    setFont(painter, 8, true);
    drawText(painter, QRect(424, y + 2, 125, 17), outcomeText(row.outcome), statusColor);
    setFont(painter, 8);
    if (!row.detail.isEmpty()) {
        const QString note = QStringLiteral("%1  ·  %2").arg(row.detail, rowOrigin(row));
        drawText(painter, QRect(46, y + 19, 503, 14), note, muted);
    }
    line(painter, y + height);
}

/// @brief Splits appendix text by measured width, including words longer than a full
/// line.
QStringList wrapLines(const QString &text, const QFontMetricsF &metrics, int width)
{
    QStringList lines;
    for (QString paragraph : text.split(QLatin1Char('\n'))) {
        paragraph = paragraph.trimmed();
        while (metrics.horizontalAdvance(paragraph) > width) {
            int fit = 1;
            while (fit < paragraph.size()
                   && metrics.horizontalAdvance(paragraph.left(fit + 1)) <= width) ++fit;
            int breakAt = paragraph.lastIndexOf(QLatin1Char(' '), fit);
            if (breakAt <= 0) breakAt = fit;
            lines.append(paragraph.left(breakAt).trimmed());
            paragraph = paragraph.mid(breakAt).trimmed();
        }
        if (!paragraph.isEmpty()) lines.append(paragraph);
    }
    return lines;
}

} // namespace

/// @brief Writes all results to A4 pages and commits the PDF only after painting
/// completes; returns an error on failure.
bool writePdf(const QString &path, const ReportMetadata &metadata,
              const QVector<TestResult> &rows, QString &error)
{
    if (rows.isEmpty()) {
        error = QStringLiteral("No tests to export.");
        return false;
    }

    QSaveFile output(path);
    if (!output.open(QIODevice::WriteOnly)) {
        error = QStringLiteral("Cannot save PDF: %1").arg(output.errorString());
        return false;
    }
    {
        QPdfWriter writer(&output);
        writer.setPageSize(QPageSize(QPageSize::A4));
        writer.setPageMargins(QMarginsF(0, 0, 0, 0), QPageLayout::Point);
        writer.setResolution(72);
        writer.setTitle(QStringLiteral("Device test report - %1").arg(metadata.serial));
        writer.setCreator(QStringLiteral("SLX Test Report"));

        QPainter painter;
        if (!painter.begin(&writer)) {
            error = QStringLiteral("Cannot generate PDF.");
            output.cancelWriting();
            return false;
        }
        painter.setRenderHint(QPainter::Antialiasing);
        int page = 1;
        drawHeader(painter, page);
        int y = drawFirstPageSummary(painter, metadata, rows.size());
        drawTableHeader(painter, y);
        y += 27;
        // Close the current page and reset the content origin on the next one.
        auto newContentPage = [&](const QString &title) {
            drawFooter(painter, page, metadata);
            if (!writer.newPage()) return false;
            ++page;
            drawHeader(painter, page);
            setFont(painter, 13, true);
            drawText(painter, QRect(left, 73, 519, 25), title);
            y = 111;
            return true;
        };
        for (int i = 0; i < rows.size(); ++i) {
            const int height = heightForRow(rows.at(i));
            if (y + height > bottom) {
                if (!newContentPage(QStringLiteral("Test results · %1").arg(metadata.serial))) {
                    painter.end();
                    error = QStringLiteral("Cannot add a PDF page.");
                    output.cancelWriting();
                    return false;
                }
                drawTableHeader(painter, y);
                y += 27;
            }
            drawRow(painter, y, i, rows.at(i), height);
            y += height;
        }
        // Appendix notes may cross page boundaries; repeat their source location
        // so every continuation remains traceable.
        setFont(painter, 8);
        const QFontMetricsF noteMetrics(painter.font());
        bool sectionStarted = false;
        for (int i = 0; i < rows.size(); ++i) {
            const auto &row = rows.at(i);
            const bool longName = noteMetrics.horizontalAdvance(row.testName) > 136;
            if (row.detail.isEmpty() && !longName) continue;
            if (!sectionStarted) {
                if (y + 62 > bottom) {
                    if (!newContentPage(QStringLiteral("Evidence notes · %1").arg(rowOrigin(row)))) {
                        painter.end();
                        error = QStringLiteral("Cannot add a PDF page.");
                        output.cancelWriting();
                        return false;
                    }
                } else {
                    y += 17;
                    line(painter, y);
                    y += 8;
                    setFont(painter, 13, true);
                    drawText(painter, QRect(left, y, 519, 22), QStringLiteral("Evidence notes"));
                    y += 31;
                }
                sectionStarted = true;
            }
            if (y + 32 > bottom) {
                if (!newContentPage(QStringLiteral("Evidence notes · %1").arg(metadata.serial))) {
                    painter.end();
                    error = QStringLiteral("Cannot add a PDF page.");
                    output.cancelWriting();
                    return false;
                }
            }
            setFont(painter, 8, true);
            drawText(painter, QRect(left, y, 519, 14),
                     QStringLiteral("TEST %1  ·  %2").arg(i + 1).arg(rowOrigin(row).toUpper()), muted);
            y += 15;
            setFont(painter, 8);
            const QStringList noteLines = wrapLines(row.testName, noteMetrics, 503)
                                          + wrapLines(row.detail, noteMetrics, 503);
            for (const QString &noteLine : noteLines) {
                if (y + 12 > bottom) {
                    if (!newContentPage(QStringLiteral("Evidence notes · %1 (continued)").arg(rowOrigin(row)))) {
                        painter.end();
                        error = QStringLiteral("Cannot add a PDF page.");
                        output.cancelWriting();
                        return false;
                    }
                    setFont(painter, 8);
                }
                drawText(painter, QRect(left + 8, y, 503, 12), noteLine,
                         noteLine == row.testName ? ink : muted);
                y += 12;
            }
            y += 8;
        }
        drawFooter(painter, page, metadata);
        if (!painter.end()) {
            error = QStringLiteral("Cannot finalize PDF.");
            output.cancelWriting();
            return false;
        }
    }
    if (!output.commit()) {
        error = QStringLiteral("Cannot complete PDF: %1").arg(output.errorString());
        return false;
    }
    error.clear();
    return true;
}

} // namespace slx
