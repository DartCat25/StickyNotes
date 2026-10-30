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

function Copy-MsvcRuntime {
    param([string]$Destination)

    $redistRoot = $null
    if ($env:VCINSTALLDIR) {
        $candidate = Join-Path $env:VCINSTALLDIR "redist\MSVC"
        if (Test-Path $candidate) {
            $redistRoot = $candidate
        }
    }

    if (-not $redistRoot) {
        $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
        if (-not (Test-Path $vswhere)) {
            $vswhere = Join-Path $env:ProgramFiles "Microsoft Visual Studio\Installer\vswhere.exe"
        }
        if (Test-Path $vswhere) {
            $vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
            if ($vsPath) {
                $candidate = Join-Path $vsPath "VC\Redist\MSVC"
                if (Test-Path $candidate) {
                    $redistRoot = $candidate
                }
            }
        }
    }

    if (-not $redistRoot) {
        throw "Visual C++ redistributable directory not found. Install the MSVC build tools."
    }

    $crtDir = Get-ChildItem $redistRoot -Directory |
        Sort-Object Name -Descending |
        ForEach-Object { Join-Path $_.FullName "x64\Microsoft.VC143.CRT" } |
        Where-Object { Test-Path $_ } |
        Select-Object -First 1

    if (-not $crtDir) {
        throw "MSVC x64 CRT directory not found under $redistRoot"
    }

    $runtimeFiles = @(
        "msvcp140.dll",
        "msvcp140_1.dll",
        "msvcp140_2.dll",
        "vcruntime140.dll",
        "vcruntime140_1.dll"
    )

    foreach ($name in $runtimeFiles) {
        $source = Join-Path $crtDir $name
        if (-not (Test-Path $source)) {
            throw "Missing MSVC runtime DLL: $name"
        }
        Copy-Item $source $Destination -Force
    }
}

function Assert-RequiredFiles {
    param([string]$Destination)

    $required = @(
        "StickyNotes.exe",
        "Qt6Core.dll",
        "Qt6Gui.dll",
        "Qt6Svg.dll",
        "Qt6Widgets.dll",
        "platforms\qwindows.dll",
        "styles\qmodernwindowsstyle.dll",
        "imageformats\qgif.dll",
        "imageformats\qico.dll",
        "imageformats\qjpeg.dll",
        "imageformats\qsvg.dll",
        "msvcp140.dll",
        "msvcp140_1.dll",
        "msvcp140_2.dll",
        "vcruntime140.dll",
        "vcruntime140_1.dll"
    )

    foreach ($relative in $required) {
        if (-not (Test-Path (Join-Path $Destination $relative))) {
            throw "Missing required runtime file: $relative"
        }
    }
}

$configureArgs = @(
    "-S", $Root,
    "-B", $BuildPath,
    "-G", "Visual Studio 17 2022",
    "-A", "x64"
)

if ($QtRoot) {
    $configureArgs += "-DCMAKE_PREFIX_PATH=$QtRoot"
}

& cmake @configureArgs
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

$windeployqt = $null
if ($QtRoot) {
    $candidate = Join-Path $QtRoot "bin\windeployqt.exe"
    if (Test-Path $candidate) {
        $windeployqt = $candidate
    }
}
if (-not $windeployqt) {
    $command = Get-Command windeployqt.exe -ErrorAction SilentlyContinue
    if ($command) {
        $windeployqt = $command.Source
    }
}
if (-not $windeployqt) {
    throw "windeployqt.exe not found. Run from a Qt-enabled shell or set QT_ROOT_DIR."
}

$deployArgs = @(
    "--release",
    "--no-compiler-runtime",
    "--no-translations",
    "--no-opengl-sw",
    "--no-system-d3d-compiler",
    "--no-system-dxc-compiler",
    "--no-ffmpeg",
    "--no-quick-import",
    "--exclude-plugins", "qsvgicon",
    (Join-Path $DistPath "StickyNotes.exe")
)

& $windeployqt @deployArgs
if ($LASTEXITCODE -ne 0) {
    throw "windeployqt failed"
}

Copy-MsvcRuntime $DistPath
Assert-RequiredFiles $DistPath

$versionLine = Select-String -Path (Join-Path $Root "CMakeLists.txt") -Pattern '^project\(StickyNotes VERSION ([^ ]+)'
$version = if ($versionLine) { $versionLine.Matches[0].Groups[1].Value } else { "1.0" }

$archive = Join-Path $Root "dist\StickyNotes-$version-windows-x64.zip"
if (Test-Path $archive) {
    Remove-Item -Force $archive
}
Compress-Archive -Path (Join-Path $DistPath "*") -DestinationPath $archive

Write-Host "Windows package: $archive"
