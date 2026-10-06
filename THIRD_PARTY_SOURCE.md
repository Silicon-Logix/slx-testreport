<!--
Silicon LogiX / SLX Test Report
Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
Author: Marco Pezzullo
License: Silicon LogiX Evaluation License 1.0. See LICENSE.
-->

# Source archives for the Windows runtime

The [v0.5.2 release](https://github.com/Silicon-Logix/slx-testreport/releases/tag/v0.5.2) offers the following source archives beside the Windows executable bundle, at no additional charge. They correspond to the unmodified Qt 6.12.0 and MinGW runtime components used for that build. Their own licenses apply; the Silicon LogiX Evaluation License does not cover them.

| Source archive | SHA-256 |
| --- | --- |
| `qtbase-everywhere-src-6.12.0.tar.xz` | `a951bd163c7b80fc6b8c88d7668fb56abf91c152373e13c10666763238131307` |
| `qtdeclarative-everywhere-src-6.12.0.tar.xz` | `311f3a2603e1973bb59baef9dfa740a376de713157d4782ee043681e889c9260` |
| `qtsvg-everywhere-src-6.12.0.tar.xz` | `e4ab39534ec97987b1b9b60ef7f4d3253d5912a69f8c713a01753174a4029331` |
| `qtshadertools-everywhere-src-6.12.0.tar.xz` | `c7d84f436e1aaef39fdcadebf2bd71bdf24dc69497e13f2e0198873cbbfea2ab` |
| `qtquick3d-everywhere-src-6.12.0.tar.xz` | `6ced6edbd25fb5a632fe770ae820b376bf50233e8d62002c79a69a505a2e429e` |
| `gcc-13.1.0.tar.xz` | `61d684f0aa5e76ac6585ad8898a2427aade8979ed5e7f85492286c4dfc13ee86` |
| `mingw-w64-v11.0.0.tar.bz2` | `bd0ea1633bd830204cc23a696889335e9d4a32b8619439ee17f22188695fcc5f` |

The Qt archives are the upstream [Qt 6.12.0 submodule releases](https://download.qt.io/archive/qt/6.12/6.12.0/submodules/). The compiler archives come from the [GNU GCC 13.1.0 release](https://ftp.gnu.org/gnu/gcc/gcc-13.1.0/) and the [MinGW-w64 11.0.0 release](https://sourceforge.net/projects/mingw-w64/files/mingw-w64/mingw-w64-release/). Silicon LogiX has not modified these components. The Windows bundle uses dynamically linked, replaceable Qt DLLs.

See [third-party notices](THIRD_PARTY_NOTICES.md) and the license and SPDX files in the Windows bundle for component terms. The application source is available in this repository under the separate [Silicon LogiX Evaluation License](LICENSE).
