#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
NATIVE="${STICKYNOTES_APPIMAGE_NATIVE:-0}"
QT_VERSION="${QT_VERSION:-6.12.0}"
QT_ARCH="${QT_ARCH:-linux_gcc_64}"
QT_DIR="${QT_DIR:-$HOME/Qt}"
QT_ROOT="${QT_ROOT:-}"
AQT_VENV="${AQT_VENV:-/tmp/aqt-venv}"

if [[ "$NATIVE" == "1" && -z "$QT_ROOT" && -n "${QT_ROOT_DIR:-}" ]]; then
    QT_ROOT="$QT_ROOT_DIR"
fi

QT_ROOT="${QT_ROOT:-$QT_DIR/$QT_VERSION/gcc_64}"

if [[ "$NATIVE" != "1" && "${STICKYNOTES_APPIMAGE_INNER:-0}" != "1" ]]; then
    if ! command -v docker >/dev/null 2>&1; then
        echo "docker is required" >&2
        exit 1
    fi

    mkdir -p "$ROOT/dist"

    exec docker run --rm \
        -e STICKYNOTES_APPIMAGE_INNER=1 \
        -e OUTPUT_DIR=/out \
        -e QT_VERSION="$QT_VERSION" \
        -e QT_ARCH="$QT_ARCH" \
        -e HOST_UID="$(id -u)" \
        -e HOST_GID="$(id -g)" \
        -v "$ROOT:/src:ro" \
        -v "$ROOT/dist:/out" \
        -w /src \
        ubuntu:22.04 \
        bash /src/packaging/build-appimage.sh
fi

OUTPUT_DIR="${OUTPUT_DIR:-$ROOT/dist}"
SUDO=()
if [[ "$NATIVE" == "1" && "$EUID" -ne 0 ]]; then
    SUDO=(sudo)
fi

export DEBIAN_FRONTEND=noninteractive

"${SUDO[@]}" env DEBIAN_FRONTEND=noninteractive apt-get update
"${SUDO[@]}" env DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    cmake \
    file \
    libegl1-mesa-dev \
    libfontconfig1-dev \
    libfreetype6-dev \
    libgl1-mesa-dev \
    libharfbuzz-dev \
    libjpeg-dev \
    libpng-dev \
    libx11-dev \
    libxcb1-dev \
    libxcb-cursor-dev \
    libxcb-icccm4-dev \
    libxcb-image0-dev \
    libxcb-keysyms1-dev \
    libxcb-randr0-dev \
    libxcb-render-util0-dev \
    libxcb-shape0-dev \
    libxcb-xinerama0-dev \
    libxcb-xkb-dev \
    libxkbcommon-dev \
    libxkbcommon-x11-dev \
    pkg-config \
    python3-venv \
    wget

TOOLS_DIR=/tmp/appimage-tools
BUILD_DIR=/tmp/stickynotes-build
APPDIR=/tmp/StickyNotes.AppDir

mkdir -p "$TOOLS_DIR"
cd "$TOOLS_DIR"

wget -q -O linuxdeploy \
    https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget -q -O linuxdeploy-plugin-qt \
    https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
wget -q -O appimagetool \
    https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage

chmod +x linuxdeploy linuxdeploy-plugin-qt appimagetool

export PATH="$TOOLS_DIR:$PATH"
export APPIMAGE_EXTRACT_AND_RUN=1
export ARCH=x86_64

if [[ ! -x "$QT_ROOT/bin/qmake" ]]; then
    if [[ ! -x "$AQT_VENV/bin/aqt" ]]; then
        python3 -m venv "$AQT_VENV"
        "$AQT_VENV/bin/pip" install --disable-pip-version-check aqtinstall==3.3.0
    fi

    "$AQT_VENV/bin/aqt" install-qt linux desktop "$QT_VERSION" "$QT_ARCH" \
        --outputdir "$QT_DIR"
fi

if [[ ! -x "$QT_ROOT/bin/qmake" ]]; then
    echo "Qt $QT_VERSION not found at $QT_ROOT" >&2
    exit 1
fi

export QMAKE="$QT_ROOT/bin/qmake"
export PATH="$QT_ROOT/bin:$PATH"
export LD_LIBRARY_PATH="$QT_ROOT/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export CMAKE_PREFIX_PATH="$QT_ROOT${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}"

"$QMAKE" -query QT_VERSION >/dev/null

cmake -S "$ROOT" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$QT_ROOT" \
    -DQt6_DIR="$QT_ROOT/lib/cmake/Qt6"
cmake --build "$BUILD_DIR" -j"$(nproc)"

rm -rf "$APPDIR"

"$TOOLS_DIR/linuxdeploy" \
    --appdir "$APPDIR" \
    --executable "$BUILD_DIR/StickyNotes" \
    --desktop-file "$ROOT/packaging/stickynotes.desktop" \
    --icon-file "$ROOT/packaging/stickynotes.png" \
    --plugin qt

if ! find "$APPDIR" -name libqxcb.so -print -quit | grep -q .; then
    echo "Qt xcb platform plugin was not bundled" >&2
    exit 1
fi

VERSION="$(sed -n 's/^project(StickyNotes VERSION \([^ ]*\).*/\1/p' "$ROOT/CMakeLists.txt" | head -n 1)"
VERSION="${VERSION:-1.0}"

OUTPUT="$TOOLS_DIR/StickyNotes-$VERSION-x86_64.AppImage"
rm -f "$OUTPUT"

"$TOOLS_DIR/appimagetool" "$APPDIR" "$OUTPUT"

mkdir -p "$OUTPUT_DIR"
install -m 0755 "$OUTPUT" "$OUTPUT_DIR/$(basename "$OUTPUT")"

if [[ -n "${HOST_UID:-}" && -n "${HOST_GID:-}" ]]; then
    chown "$HOST_UID:$HOST_GID" "$OUTPUT_DIR/$(basename "$OUTPUT")"
fi

echo "AppImage: $OUTPUT_DIR/$(basename "$OUTPUT")"
