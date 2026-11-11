#!/usr/bin/env bash
set -euo pipefail

# Downloads the AppImage tooling used by packaging/build-appimage-inner.sh.
#
# Every download is bounded: a stalled transfer is aborted and retried instead
# of hanging forever (plain `wget` without timeouts is what used to hang the CI
# build on the linuxdeploy release download).
#
# Usage: install-appimage-tools.sh [TOOL_DIR]
# The directory defaults to $APPIMAGE_TOOLS_DIR or
# ${XDG_CACHE_HOME:-$HOME/.cache}/stickynotes-appimage-tools.

LINUXDEPLOY_VERSION="1-alpha-20251107-1"
LINUXDEPLOY_QT_VERSION="1-alpha-20250213-1"
# appimagetool only publishes a rolling "continuous" release.
APPIMAGETOOL_VERSION="continuous"

TOOLS_DIR="${1:-${APPIMAGE_TOOLS_DIR:-${XDG_CACHE_HOME:-$HOME/.cache}/stickynotes-appimage-tools}}"

if ! command -v curl >/dev/null 2>&1; then
    echo "curl is required to download the AppImage tools" >&2
    exit 1
fi

mkdir -p "$TOOLS_DIR"

is_appimage() {
    # AppImage files are ELF executables. A truncated download or an HTML error
    # page must never be treated as a usable tool.
    [[ -f "$1" ]] && [[ "$(head -c 4 "$1" | od -An -tx1 | tr -d ' \n')" == "7f454c46" ]]
}

fetch_tool() {
    local name="$1"
    local url="$2"
    local target="$TOOLS_DIR/$name"
    local tmp="$TOOLS_DIR/$name.part"

    if is_appimage "$target"; then
        echo "$name: already available in $TOOLS_DIR"
        return 0
    fi

    echo "$name: downloading $url"
    rm -f "$tmp"
    curl \
        --fail \
        --location \
        --silent \
        --show-error \
        --connect-timeout 15 \
        --max-time 300 \
        --retry 5 \
        --retry-all-errors \
        --retry-delay 5 \
        --speed-limit 1024 \
        --speed-time 60 \
        --output "$tmp" \
        "$url"

    if ! is_appimage "$tmp"; then
        rm -f "$tmp"
        echo "$name: downloaded file is not an AppImage" >&2
        exit 1
    fi

    chmod +x "$tmp"
    mv -f "$tmp" "$target"
    echo "$name: ready at $target"
}

fetch_tool linuxdeploy \
    "https://github.com/linuxdeploy/linuxdeploy/releases/download/$LINUXDEPLOY_VERSION/linuxdeploy-x86_64.AppImage"
fetch_tool linuxdeploy-plugin-qt \
    "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/$LINUXDEPLOY_QT_VERSION/linuxdeploy-plugin-qt-x86_64.AppImage"
fetch_tool appimagetool \
    "https://github.com/AppImage/appimagetool/releases/download/$APPIMAGETOOL_VERSION/appimagetool-x86_64.AppImage"

echo "AppImage tools are available in $TOOLS_DIR"
