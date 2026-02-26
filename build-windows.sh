#!/bin/bash
set -e

MXE_DIR="/media/data/999-OS/Documents/mxe"
PROJECT_DIR="$(pwd)"
DIST_DIR="${PROJECT_DIR}/dist/windows"

mkdir -p "$DIST_DIR"

docker build -t nohboard-windows-builder -f Dockerfile.windows .

docker run --rm -it \
    -v "$MXE_DIR":"$MXE_DIR":ro \
    -v "$PROJECT_DIR":/app \
    -v "$DIST_DIR":/dist \
    -w /app \
    -e MXE_DIR="$MXE_DIR" \
    nohboard-windows-builder \
    bash -c "export PATH=${MXE_DIR}/usr/bin:\$PATH && \
             export CCACHE_DISABLE=1 && \
             rm -rf build && mkdir build && cd build && \
             x86_64-w64-mingw32.static-cmake .. -DCMAKE_BUILD_TYPE=Release && \
             make -j\$(nproc) && \
             cp NohBoard.exe /dist/"

echo "✅ Windows executable copied to ./dist/windows/NohBoard.exe"