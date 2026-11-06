#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUTPUT_DIR="${OUTPUT_DIR:-$ROOT/dist}"
BUILD_DIR="${BUILD_DIR:-/tmp/stickynotes-build}"
APPDIR="${APPDIR:-/tmp/StickyNotes.AppDir}"

for tool in cmake qmake6 linuxdeploy linuxdeploy-plugin-qt appimagetool; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "$tool not found. Use packaging/build-appimage.sh (Docker) or install the packages from packaging/Dockerfile.appimage." >&2
        exit 1
    fi
done

export QMAKE="$(command -v qmake6)"
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
