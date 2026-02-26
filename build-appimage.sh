#!/bin/bash
set -e

docker build -t nohboard-appimage-builder .

mkdir -p dist
docker run --rm -v $(pwd)/dist:/dist nohboard-appimage-builder \
    bash -c "cp /app/build/NohBoard-*.AppImage /dist/linux"

echo "✅ AppImage created in ./dist/"