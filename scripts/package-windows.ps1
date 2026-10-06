# Silicon LogiX / SLX Test Report
# Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
# Author: Marco Pezzullo
# License: Silicon LogiX Evaluation License 1.0. See LICENSE.

# Stages the Windows executable with Qt, MinGW, examples, and notices.

param(
    [string]$QtRoot,
    [string]$ToolchainBin,
    [string]$BuildDirectory = 'build\windows-mingw',
    [string]$OutputDirectory = 'dist\SLX-Test-Report',
    [switch]$CreateArchive
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$projectRoot = [System.IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))

# Resolve caller-supplied paths against the repository, independent of the
# shell's working directory.
function Resolve-ProjectPath([string]$Path) {
    if ([System.IO.Path]::IsPathRooted($Path)) {
        return [System.IO.Path]::GetFullPath($Path)
    }
    return [System.IO.Path]::GetFullPath((Join-Path $projectRoot $Path))
}

# Reject output, staging, and cleanup paths outside the repository or through
# linked directories. External build directories remain read-only inputs.
function Assert-WorkspaceTarget([string]$Path) {
    $fullPath = [System.IO.Path]::GetFullPath($Path)
    $prefix = $projectRoot.TrimEnd('\', '/') + [System.IO.Path]::DirectorySeparatorChar
    if (-not $fullPath.StartsWith($prefix, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Package target must be inside the project: $fullPath"
    }
    $segmentPath = $projectRoot
    foreach ($segment in $fullPath.Substring($prefix.Length).Split([char[]]@('\', '/'), [System.StringSplitOptions]::RemoveEmptyEntries)) {
        $segmentPath = Join-Path $segmentPath $segment
        if ((Test-Path -LiteralPath $segmentPath) -and
            ((Get-Item -LiteralPath $segmentPath).Attributes -band [System.IO.FileAttributes]::ReparsePoint)) {
            throw "Package target crosses a linked path: $segmentPath"
        }
    }
    return $fullPath
}

# Read one exact CMake cache value; return null when the key is absent.
function Read-CMakeCache([string]$Name) {
    $pattern = '^' + [regex]::Escape($Name) + ':[^=]*=(.*)$'
    $match = Select-String -LiteralPath $cachePath -Pattern $pattern | Select-Object -First 1
    if ($null -eq $match) { return $null }
    return $match.Matches[0].Groups[1].Value
}

$buildPath = Resolve-ProjectPath $BuildDirectory
$cachePath = Join-Path $buildPath 'CMakeCache.txt'
$sourceExe = Join-Path $buildPath 'slx-test-report.exe'
$outputPath = Assert-WorkspaceTarget (Resolve-ProjectPath $OutputDirectory)
$outputParent = Split-Path -Parent $outputPath
$qmlDirectory = Join-Path $projectRoot 'qml'

foreach ($requiredPath in @($cachePath, $sourceExe, $qmlDirectory)) {
    if (-not (Test-Path -LiteralPath $requiredPath)) {
        throw "Required build file or directory not found: $requiredPath"
    }
}

if (-not $QtRoot) { $QtRoot = $env:QT_ROOT }
if (-not $QtRoot) {
    $qtConfig = Read-CMakeCache 'Qt6_DIR'
    if (-not $qtConfig) { throw 'Qt installation not found in the CMake cache. Pass -QtRoot.' }
    $QtRoot = [System.IO.Path]::GetFullPath((Join-Path $qtConfig '..\..\..'))
}
$QtRoot = [System.IO.Path]::GetFullPath($QtRoot)
$qtBin = Join-Path $QtRoot 'bin'
$deployTool = Join-Path $qtBin 'windeployqt.exe'
$compiler = Read-CMakeCache 'CMAKE_CXX_COMPILER'
if (-not $ToolchainBin -and $compiler) { $ToolchainBin = Split-Path -Parent $compiler }
if (-not $ToolchainBin) { throw 'MinGW compiler path not found. Pass -ToolchainBin.' }
$ToolchainBin = [System.IO.Path]::GetFullPath($ToolchainBin)
if ($ToolchainBin -notmatch 'mingw') {
    throw 'This bundle script supports the Qt MinGW kit. Use a separate MSVC redistribution workflow.'
}
foreach ($requiredPath in @($deployTool, $ToolchainBin)) {
    if (-not (Test-Path -LiteralPath $requiredPath)) { throw "Required Qt tool not found: $requiredPath" }
}

$toolchainRoot = Split-Path -Parent $ToolchainBin
$qmake = Join-Path $qtBin 'qmake.exe'
$gxx = Join-Path $ToolchainBin 'g++.exe'
$stripTool = Join-Path $ToolchainBin 'strip.exe'
$buildInfo = Join-Path $toolchainRoot 'build-info.txt'
foreach ($requiredPath in @($qmake, $gxx, $stripTool, $buildInfo)) {
    if (-not (Test-Path -LiteralPath $requiredPath)) { throw "Release tool not found: $requiredPath" }
}
$qtVersion = (& $qmake -query QT_VERSION | Select-Object -First 1).Trim()
$gccVersion = (& $gxx -dumpfullversion | Select-Object -First 1).Trim()
if ($qtVersion -ne '6.12.0' -or $gccVersion -ne '13.1.0' -or
    (Get-Content -LiteralPath $buildInfo -Raw) -notmatch 'rt-version=v11') {
    throw 'This release bundle requires Qt 6.12.0 and MinGW GCC 13.1.0 with MinGW-w64 v11; the published source manifest must match its DLLs.'
}

$runtimeLicenses = @{
    'GCC-GPLv3.txt' = 'licenses\gcc\COPYING3'
    'GCC-runtime-exception.txt' = 'licenses\gcc\COPYING.RUNTIME'
    'Winpthreads-LICENSE.txt' = 'licenses\winpthreads\COPYING'
    'MinGW-w64-runtime-LICENSE.txt' = 'licenses\mingw-w64\COPYING.MinGW-w64-runtime.txt'
}
foreach ($relativePath in $runtimeLicenses.Values) {
    if (-not (Test-Path -LiteralPath (Join-Path $toolchainRoot $relativePath))) {
        throw "Compiler runtime license is missing: $relativePath"
    }
}

New-Item -ItemType Directory -Path $outputParent -Force | Out-Null
$stagePath = Assert-WorkspaceTarget (Join-Path $outputParent ('.slx-stage-' + [guid]::NewGuid().ToString('N')))
$backupPath = Assert-WorkspaceTarget (Join-Path $outputParent ('.slx-backup-' + [guid]::NewGuid().ToString('N')))
$originalPath = $env:PATH

try {
    New-Item -ItemType Directory -Path $stagePath | Out-Null
    $stageExe = Join-Path $stagePath 'slx-test-report.exe'
    Copy-Item -LiteralPath $sourceExe -Destination $stageExe
    # Qt's static Windows entry point carries debug sections even in Release builds.
    # Remove them from the staged copy, including its original build-machine paths.
    & $stripTool --strip-debug $stageExe
    if ($LASTEXITCODE -ne 0) { throw "Removing debug metadata failed (exit code $LASTEXITCODE)." }
    $env:PATH = "$qtBin;$ToolchainBin;$originalPath"
    & $deployTool --release --compiler-runtime --no-translations --no-system-d3d-compiler --no-opengl-sw --verbose 0 --qmldir $qmlDirectory --dir $stagePath $stageExe
    if ($LASTEXITCODE -ne 0) { throw "windeployqt failed (exit code $LASTEXITCODE)." }

    # The application selects Basic before loading QML, so other control
    # styles and QML debugging plugins are excluded from the release.
    foreach ($style in @('FluentWinUI3', 'Fusion', 'Imagine', 'Material', 'Universal', 'Windows')) {
        $stylePath = Assert-WorkspaceTarget (Join-Path $stagePath (Join-Path 'qml\QtQuick\Controls' $style))
        if (Test-Path -LiteralPath $stylePath) { Remove-Item -LiteralPath $stylePath -Recurse -Force }
        foreach ($library in (Get-ChildItem -LiteralPath $stagePath -Filter "Qt6QuickControls2${style}*.dll" -File)) {
            Remove-Item -LiteralPath (Assert-WorkspaceTarget $library.FullName) -Force
        }
    }
    foreach ($unusedPath in @('qmltooling', 'qml\QtQuick\tooling')) {
        $target = Assert-WorkspaceTarget (Join-Path $stagePath $unusedPath)
        if (Test-Path -LiteralPath $target) { Remove-Item -LiteralPath $target -Recurse -Force }
    }

    foreach ($name in @('LICENSE', 'README.md', 'THIRD_PARTY_NOTICES.md', 'THIRD_PARTY_SOURCE.md')) {
        Copy-Item -LiteralPath (Join-Path $projectRoot $name) -Destination $stagePath
    }
    $examplesPath = Join-Path $stagePath 'examples'
    New-Item -ItemType Directory -Path $examplesPath | Out-Null
    foreach ($name in @('demo.csv', 'demo-report.pdf', 'demo-profile.json',
                       'batch.csv', 'batch-profile.json')) {
        Copy-Item -LiteralPath (Join-Path $projectRoot (Join-Path 'examples' $name)) -Destination $examplesPath
    }
    $screenshotsPath = Join-Path $stagePath 'assets\screenshots'
    New-Item -ItemType Directory -Path $screenshotsPath -Force | Out-Null
    foreach ($name in @('overview.png', 'report-preview.png', 'csv-editor.png')) {
        Copy-Item -LiteralPath (Join-Path $projectRoot (Join-Path 'assets\screenshots' $name)) -Destination $screenshotsPath
    }

    $licensePath = Join-Path $stagePath 'licenses'
    New-Item -ItemType Directory -Path $licensePath | Out-Null
    Copy-Item -LiteralPath (Join-Path $projectRoot 'licenses\Qt-Open-Source-LICENSE.txt') -Destination $licensePath
    foreach ($name in $runtimeLicenses.Keys) {
        Copy-Item -LiteralPath (Join-Path $toolchainRoot $runtimeLicenses[$name]) -Destination (Join-Path $licensePath $name)
    }
    $sbomPath = Join-Path $QtRoot 'sbom'
    if (Test-Path -LiteralPath $sbomPath) {
        $sbomOutput = Join-Path $licensePath 'qt-sbom'
        New-Item -ItemType Directory -Path $sbomOutput | Out-Null
        foreach ($sbom in (Get-ChildItem -LiteralPath $sbomPath -Filter '*.spdx' -File |
                           Where-Object { $_.Name -match '^(qtbase|qtdeclarative|qtsvg|qtshadertools|qtquick3d)-' })) {
            Copy-Item -LiteralPath $sbom.FullName -Destination $sbomOutput
        }
    }

    if (Test-Path -LiteralPath $outputPath) {
        if ((Get-Item -LiteralPath $outputPath).Attributes -band [System.IO.FileAttributes]::ReparsePoint) {
            throw "Refusing to replace a linked output directory: $outputPath"
        }
        Move-Item -LiteralPath $outputPath -Destination $backupPath
    }
    try {
        Move-Item -LiteralPath $stagePath -Destination $outputPath
    } catch {
        if (Test-Path -LiteralPath $backupPath) {
            Move-Item -LiteralPath $backupPath -Destination $outputPath
        }
        throw
    }
    if (Test-Path -LiteralPath $backupPath) {
        Remove-Item -LiteralPath $backupPath -Recurse -Force
    }

    if ($CreateArchive) {
        $archivePath = Assert-WorkspaceTarget (Join-Path $outputParent 'SLX-Test-Report-win64.zip')
        Compress-Archive -LiteralPath $outputPath -DestinationPath $archivePath -CompressionLevel Optimal -Force
        Write-Output "Archive ready: $archivePath"
    }
    Write-Output "Application ready: $(Join-Path $outputPath 'slx-test-report.exe')"
} finally {
    $env:PATH = $originalPath
    if (Test-Path -LiteralPath $stagePath) {
        Remove-Item -LiteralPath $stagePath -Recurse -Force
    }
}
