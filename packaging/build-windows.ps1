param(
    [string]$Config = "Release",
    [string]$BuildDir = "build-windows",
    [string]$DistDir = "dist/windows"
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$BuildPath = Join-Path $Root $BuildDir
$DistPath = Join-Path $Root $DistDir
$QtRoot = if ($env:QT_ROOT_DIR) { $env:QT_ROOT_DIR } elseif ($env:QTDIR) { $env:QTDIR } else { $null }

if (-not $QtRoot) {
    throw "Set QT_ROOT_DIR or QTDIR to the Qt installation directory."
}

& cmake -S $Root -B $BuildPath -G "Visual Studio 17 2022" -A x64 "-DCMAKE_PREFIX_PATH=$QtRoot"
if ($LASTEXITCODE -ne 0) {
    throw "cmake configure failed"
}

& cmake --build $BuildPath --config $Config --parallel
if ($LASTEXITCODE -ne 0) {
    throw "cmake build failed"
}

$exe = Join-Path $BuildPath "$Config\StickyNotes.exe"
if (-not (Test-Path $exe)) {
    throw "executable not found: $exe"
}

if (Test-Path $DistPath) {
    Remove-Item -Recurse -Force $DistPath
}
New-Item -ItemType Directory -Force -Path $DistPath | Out-Null
Copy-Item $exe (Join-Path $DistPath "StickyNotes.exe")

$windeployqt = Join-Path $QtRoot "bin\windeployqt.exe"
if (-not (Test-Path $windeployqt)) {
    throw "windeployqt.exe not found in $QtRoot"
}

& $windeployqt --release --no-compiler-runtime --no-translations --no-opengl-sw `
    --no-system-d3d-compiler --no-quick-import (Join-Path $DistPath "StickyNotes.exe")
if ($LASTEXITCODE -ne 0) {
    throw "windeployqt failed"
}

$redistRoot = if ($env:VCToolsRedistDir) { $env:VCToolsRedistDir } else { Join-Path $env:VCINSTALLDIR "redist\MSVC" }
$crtDir = Get-ChildItem $redistRoot -Recurse -Directory -Filter "Microsoft.VC*.CRT" |
    Where-Object { $_.FullName -match '\\x64\\' } |
    Select-Object -First 1
if (-not $crtDir) {
    throw "MSVC x64 CRT directory not found under $redistRoot"
}
Copy-Item (Join-Path $crtDir.FullName "*.dll") $DistPath -Force

$required = @(
    "StickyNotes.exe",
    "Qt6Core.dll",
    "Qt6Gui.dll",
    "Qt6Widgets.dll",
    "platforms\qwindows.dll",
    "msvcp140.dll",
    "vcruntime140.dll"
)
foreach ($relative in $required) {
    if (-not (Test-Path (Join-Path $DistPath $relative))) {
        throw "Missing required runtime file: $relative"
    }
}

$versionMatch = Select-String -Path (Join-Path $Root "CMakeLists.txt") -Pattern '^project\(StickyNotes VERSION ([^ ]+)'
$version = if ($versionMatch) { $versionMatch.Matches[0].Groups[1].Value } else { "1.0" }

$archive = Join-Path $Root "dist\StickyNotes-$version-windows-x64.zip"
if (Test-Path $archive) {
    Remove-Item -Force $archive
}
Compress-Archive -Path (Join-Path $DistPath "*") -DestinationPath $archive

Write-Host "Windows package: $archive"
