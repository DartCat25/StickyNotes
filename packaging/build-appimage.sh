#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUTPUT_DIR="${OUTPUT_DIR:-$ROOT/dist}"
IMAGE="${STICKYNOTES_APPIMAGE_IMAGE:-stickynotes-appimage}"

# Native mode: the current environment already has the packages and tools from
# packaging/Dockerfile.appimage. Used by CI or a prepared dev machine.
if [[ "${STICKYNOTES_APPIMAGE_NATIVE:-0}" == "1" ]]; then
    exec bash "$ROOT/packaging/build-appimage-inner.sh"
fi

if ! command -v docker >/dev/null 2>&1; then
    echo "docker is required. Set STICKYNOTES_APPIMAGE_NATIVE=1 to use a prepared environment." >&2
    exit 1
fi

mkdir -p "$OUTPUT_DIR"
OUTPUT_DIR="$(cd "$OUTPUT_DIR" && pwd)"

docker build -t "$IMAGE" -f "$ROOT/packaging/Dockerfile.appimage" "$ROOT/packaging"

exec docker run --rm \
    -e OUTPUT_DIR=/out \
    -e HOST_UID="$(id -u)" \
    -e HOST_GID="$(id -g)" \
    -v "$ROOT:/src:ro" \
    -v "$OUTPUT_DIR:/out" \
    -w /src \
    "$IMAGE" \
    bash /src/packaging/build-appimage-inner.sh
