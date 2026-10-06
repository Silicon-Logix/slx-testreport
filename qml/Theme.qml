// Silicon LogiX / SLX Test Report
// Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.

// Shared colors for result rows, analysis charts, and the report preview.

import QtQuick

QtObject {
    readonly property color ink: "#18203B"
    readonly property color muted: "#69738C"
    readonly property color accent: "#3436DC"
    readonly property color violet: "#B02FE2"
    readonly property color navy: "#111832"
    readonly property color canvas: "#F3F5FB"
    readonly property color lineColor: "#DFE4F0"
    readonly property color success: "#167565"
    readonly property color failure: "#B4484E"
    readonly property color attention: "#9B691B"
    readonly property color skipped: "#687695"

    // Keep outcome colors consistent across the table, charts, and preview.
    function statusColor(code) {
        return code === "pass" ? success : code === "fail" ? failure
             : code === "skipped" ? skipped : attention
    }
}
