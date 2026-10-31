#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
NATIVE="${STICKYNOTES_APPIMAGE_NATIVE:-0}"

if [[ "$NATIVE" != "1" && "${STICKYNOTES_APPIMAGE_INNER:-0}" != "1" ]]; then
    if ! command -v docker >/dev/null 2>&1; then
        echo "docker is required" >&2
        exit 1
    fi

    mkdir -p "$ROOT/dist"

    exec docker run --rm \
        -e STICKYNOTES_APPIMAGE_INNER=1 \
        -e OUTPUT_DIR=/out \
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
    libfuse2 \
    libgl1-mesa-dev \
    libx11-dev \
    libxcb-cursor0 \
    libxcb1-dev \
    pkg-config \
    qt6-base-dev \
    qt6-base-dev-tools \
    qt6-declarative-dev \
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
export QMAKE="${QMAKE:-$(command -v qmake6 || true)}"

if [[ -z "$QMAKE" ]]; then
    QMAKE=/usr/lib/qt6/bin/qmake6
fi

cmake -S "$ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
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
