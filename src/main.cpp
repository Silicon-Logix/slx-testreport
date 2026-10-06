// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

/// @file
/// @brief Starts the Qt application and binds report services to QML.

#include "csvcomposer.h"
#include "reportcontroller.h"
#include "resultsfiltermodel.h"

#include <QFont>
#include <QFile>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickStyle>
#include <QTimer>
#include <QUrl>

/// @brief Initializes application state and QML, then enters the UI loop or completes
/// the requested startup probe.
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setOrganizationName(QStringLiteral("Silicon LogiX"));
    QGuiApplication::setApplicationName(QStringLiteral("SLX Test Report"));
    QGuiApplication::setApplicationVersion(QString::fromLatin1(SLX_APP_VERSION));
    QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/brand/report-icon.svg")));
    app.setFont(QFont(QStringLiteral("Segoe UI")));
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    ReportController report;
    CsvComposer csvComposer;
    ResultsFilterModel resultsFilter;
    resultsFilter.setSourceModel(report.resultsModel());
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("report"), &report);
    engine.rootContext()->setContextProperty(QStringLiteral("csvComposer"), &csvComposer);
    engine.rootContext()->setContextProperty(QStringLiteral("filteredResultsModel"), &resultsFilter);
    engine.rootContext()->setContextProperty(QStringLiteral("appVersion"), app.applicationVersion());
    QFile license(QStringLiteral(":/legal/LICENSE"));
    if (!license.open(QIODevice::ReadOnly)) return 1;
    engine.rootContext()->setContextProperty(QStringLiteral("embeddedLicenseText"),
                                             QString::fromUtf8(license.readAll()));
    QFile notices(QStringLiteral(":/legal/third-party-notices"));
    if (!notices.open(QIODevice::ReadOnly)) return 1;
    QString noticesText = QString::fromUtf8(notices.readAll());
    if (noticesText.startsWith(QStringLiteral("<!--"))) {
        const qsizetype headerEnd = noticesText.indexOf(QStringLiteral("-->"));
        if (headerEnd >= 0) noticesText.remove(0, headerEnd + 3);
    }
    engine.rootContext()->setContextProperty(QStringLiteral("embeddedThirdPartyNotices"),
                                             noticesText.trimmed());
    QFile qtLicense(QStringLiteral(":/legal/qt-license"));
    if (!qtLicense.open(QIODevice::ReadOnly)) return 1;
    engine.rootContext()->setContextProperty(QStringLiteral("embeddedQtLicenseText"),
                                             QString::fromUtf8(qtLicense.readAll()));
    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    if (engine.rootObjects().isEmpty()) return 1;
    // Load the controller and QML tree before accepting --smoke-test as a
    // successful startup; skip only the interactive event loop.
    if (app.arguments().contains(QStringLiteral("--smoke-test"))) return 0;
    // --snapshot captures a repeatable example state for visual inspection.
    const int snapshotArg = app.arguments().indexOf(QStringLiteral("--snapshot"));
    if (snapshotArg >= 0) {
        if (snapshotArg + 1 >= app.arguments().size()) return 2;
        if (app.arguments().contains(QStringLiteral("--batch"))) report.loadBatchExample();
        else report.loadExample();
        if (app.arguments().contains(QStringLiteral("--second-device"))) report.selectDevice(1);
        if (app.arguments().contains(QStringLiteral("--compact"))) {
            auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
            if (!window) return 4;
            window->resize(1060, 680);
        }
        if (app.arguments().contains(QStringLiteral("--preview"))) {
            QObject *workspace = engine.rootObjects().first()->findChild<QObject *>(QStringLiteral("slxReportWorkspace"));
            if (!workspace || !workspace->setProperty("currentTab", 1)) return 4;
        }
        const QString dialogName = app.arguments().contains(QStringLiteral("--license"))
            ? QStringLiteral("slxLicenseDialog")
            : app.arguments().contains(QStringLiteral("--notices"))
                  ? QStringLiteral("slxNoticesDialog")
            : app.arguments().contains(QStringLiteral("--qt-license"))
                  ? QStringLiteral("slxQtLicenseDialog")
            : app.arguments().contains(QStringLiteral("--composer"))
                  ? QStringLiteral("slxCsvComposerDialog")
            : app.arguments().contains(QStringLiteral("--about"))
                  ? QStringLiteral("slxAboutDialog") : QString();
        if (!dialogName.isEmpty()) {
            QObject *dialog = engine.rootObjects().first()->findChild<QObject *>(dialogName);
            if (!dialog || !QMetaObject::invokeMethod(dialog, "open")) return 4;
        }
        const QString target = app.arguments().at(snapshotArg + 1);
        // Capture only after the first frame has reached the window.
        QTimer::singleShot(1000, &app, [&app, &engine, target] {
            auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
            app.exit(window && window->grabWindow().save(target) ? 0 : 3);
        });
    }
    return app.exec();
}
