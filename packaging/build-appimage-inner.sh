#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUTPUT_DIR="${OUTPUT_DIR:-$ROOT/dist}"
BUILD_DIR="${BUILD_DIR:-/tmp/stickynotes-build}"
APPDIR="${APPDIR:-/tmp/StickyNotes.AppDir}"
TOOLS_DIR="${APPIMAGE_TOOLS_DIR:-${XDG_CACHE_HOME:-$HOME/.cache}/stickynotes-appimage-tools}"

# Inside the Docker image the tools are already installed in /opt/appimage-tools
# (see packaging/Dockerfile.appimage). On a native build host they are fetched
# once into TOOLS_DIR.
missing_tools=()
for tool in linuxdeploy linuxdeploy-plugin-qt appimagetool; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        missing_tools+=("$tool")
    fi
done
if (( ${#missing_tools[@]} > 0 )); then
    bash "$ROOT/packaging/install-appimage-tools.sh" "$TOOLS_DIR"
    export PATH="$TOOLS_DIR:$PATH"
fi

missing=()
for tool in cmake g++ patchelf pkg-config linuxdeploy linuxdeploy-plugin-qt appimagetool; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        missing+=("$tool")
    fi
done
if (( ${#missing[@]} > 0 )); then
    echo "Missing build tools: ${missing[*]}" >&2
    echo "Install the packages listed in packaging/Dockerfile.appimage, or build in Docker with STICKYNOTES_APPIMAGE_DOCKER=1." >&2
    exit 1
fi

# linuxdeploy-plugin-qt needs a Qt 6 qmake; never fall back to Qt 5.
QMAKE_BIN=""
for candidate in qmake6 qmake-qt6 qmake; do
    if command -v "$candidate" >/dev/null 2>&1 \
        && [[ "$("$candidate" -query QT_VERSION 2>/dev/null)" == 6.* ]]; then
        QMAKE_BIN="$(command -v "$candidate")"
        break
    fi
done
if [[ -z "$QMAKE_BIN" ]]; then
    echo "Qt 6 qmake not found. Install qt6-base-dev (Debian/Ubuntu) or build in Docker with STICKYNOTES_APPIMAGE_DOCKER=1." >&2
    exit 1
fi

export QMAKE="$QMAKE_BIN"
export APPIMAGE_EXTRACT_AND_RUN=1
export ARCH=x86_64

rm -rf "$BUILD_DIR" "$APPDIR"
cmake -S "$ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" -j"$(nproc)"

linuxdeploy \
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

mkdir -p "$OUTPUT_DIR"
OUTPUT="$OUTPUT_DIR/StickyNotes-$VERSION-x86_64.AppImage"
rm -f "$OUTPUT"
appimagetool "$APPDIR" "$OUTPUT"

# In Docker the file is created by root; give it back to the host user.
if [[ -n "${HOST_UID:-}" && -n "${HOST_GID:-}" ]]; then
    chown "$HOST_UID:$HOST_GID" "$OUTPUT" 2>/dev/null || true
fi

echo "AppImage: $OUTPUT"
