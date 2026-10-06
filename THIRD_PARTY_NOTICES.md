<!--
Silicon LogiX / SLX Test Report
Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
Author: Marco Pezzullo
License: Silicon LogiX Evaluation License 1.0. See LICENSE.
-->

# Third-party notices

SLX Test Report uses Qt 6 Core, GUI, QML/Quick, Quick Controls, Quick Dialogs and SVG. The Windows bundle links to Qt through replaceable DLLs; its Qt modules, QML imports, plugins and bundled third-party code retain their own licenses. The [Qt license terms](licenses/Qt-Open-Source-LICENSE.txt) accompany this source tree and the Windows bundle. Qt documents its [license options](https://doc.qt.io/qt-6/licensing.html), [third-party components](https://doc.qt.io/qt-6/licenses-used-in-qt.html) and [SBOM format](https://doc.qt.io/qt-6/sbom.html). The bundle also includes the available SPDX files for the Qt modules used by this build.

The corresponding source archives for the Windows runtime are listed in [third-party source availability](https://github.com/Silicon-Logix/slx-testreport/blob/main/THIRD_PARTY_SOURCE.md) and attached to the same release as the executable bundle.

The MinGW Windows bundle includes GCC runtime DLLs and winpthreads. Their copyright and license texts are copied into its `licenses/` directory from the compiler installation: GPLv3, the GCC Runtime Library Exception, the winpthreads notice and the MinGW-w64 runtime notices. [GCC explains the libstdc++ terms](https://gcc.gnu.org/onlinedocs/libstdc%2B%2B/manual/license.html).

The [Silicon LogiX Evaluation License](LICENSE) applies only to the material identified in that license. It does not restrict rights granted by Qt, GCC, MinGW-w64 or their third-party components. Users may replace the dynamically linked Qt libraries in the Windows bundle.
