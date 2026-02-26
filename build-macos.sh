#!/bin/bash
set -e

PROJECT_DIR="$(pwd)"
DIST_DIR="${PROJECT_DIR}/dist/macos"
UNIVERSAL_APP="NohBoard.app"

mkdir -p "$DIST_DIR"

# Build the Docker image (cached)
docker build -t nohboard-macos-builder -f Dockerfile.macos .

# Run as root to ensure write access to /dist
docker run --rm -it --user root \
    -v "$PROJECT_DIR":/app:ro \
    -v "$DIST_DIR":/dist \
    nohboard-macos-builder \
    bash -c "
        set -e
        # Use a temporary build directory inside the container
        BUILD_DIR=\$(mktemp -d)
        cd \$BUILD_DIR
        cp -r /app/* .

        # Build for x86_64 as an app bundle (let CMake create the .app)
        mkdir build-x86_64 && cd build-x86_64
        qt-cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=x86_64 -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
        cmake --build . --parallel
        cd ..

        # Build for arm64 as an app bundle
        mkdir build-arm64 && cd build-arm64
        qt-cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
        cmake --build . --parallel
        cd ..

        # Now we have two app bundles: build-x86_64/NohBoard.app and build-arm64/NohBoard.app
        # Extract the executables
        EXEC_X86_64="build-x86_64/NohBoard.app/Contents/MacOS/NohBoard"
        EXEC_ARM64="build-arm64/NohBoard.app/Contents/MacOS/NohBoard"

        # Create a new universal app bundle (base it on the x86_64 one)
        cp -r build-x86_64/NohBoard.app ${UNIVERSAL_APP}

        # Combine executables with lipo
        LIPO=\$(find /opt -name '*-lipo' -type f 2>/dev/null | head -1)
        if [ -z \"\$LIPO\" ]; then
            echo 'ERROR: lipo not found in container'
            exit 1
        fi
        echo \"Using lipo: \$LIPO\"
        \$LIPO -create \"\$EXEC_X86_64\" \"\$EXEC_ARM64\" -output \"${UNIVERSAL_APP}/Contents/MacOS/NohBoard\"

        # Copy keyboards into Resources (if not already there – the original bundle might not have them)
        mkdir -p ${UNIVERSAL_APP}/Contents/Resources/keyboards
        cp -r keyboards/* ${UNIVERSAL_APP}/Contents/Resources/keyboards/

        # Run macdeployqt to adjust frameworks and ensure Info.plist is correct
        macdeployqt ${UNIVERSAL_APP} -verbose=1

        # Copy result to dist
        cp -r ${UNIVERSAL_APP} /dist/

        # Clean up
        rm -rf \$BUILD_DIR
        echo '=== Universal app bundle created ==='
    "

echo "✅ Universal macOS app bundle created at ./dist/macos/NohBoard.app"