#!/usr/bin/env bash
# Build Blender 2.79b as .so for Android ARM64
# Uses NDK cross-compilation + GL4ES for OpenGL→GLES translation
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"
OUTPUT_DIR="$PROJECT_DIR/output"
BLENDER_DIR="$PROJECT_DIR/blender279"
GL4ES_DIR="$PROJECT_DIR/gl4es"
PATCHES_DIR="$PROJECT_DIR/patches"
ANDROID_API="${ANDROID_API:-24}"
ARCH="${ARCH:-arm64-v8a}"
JOBS="${JOBS:-$(nproc)}"
NDK_DIR="${ANDROID_NDK_HOME:-$ANDROID_NDK_ROOT}"

if [ -z "$NDK_DIR" ] || [ ! -d "$NDK_DIR" ]; then
	echo "ERROR: Android NDK not found. Set ANDROID_NDK_HOME"
	exit 1
fi

TOOLCHAIN="$NDK_DIR/build/cmake/android.toolchain.cmake"
if [ ! -f "$TOOLCHAIN" ]; then
	echo "ERROR: CMake toolchain not found at $TOOLCHAIN"
	exit 1
fi

echo "=== Building Blender 2.79b for Android ==="
echo "ARCH: $ARCH"
echo "NDK: $NDK_DIR"
echo "Jobs: $JOBS"

# ---- Step 1: Clone sources if needed ----
if [ ! -d "$BLENDER_DIR" ]; then
	echo "Cloning Blender 2.79b..."
	git clone --depth 1 --branch v2.79b https://github.com/blender/blender.git "$BLENDER_DIR"
fi

if [ ! -d "$GL4ES_DIR" ]; then
	echo "Cloning GL4ES..."
	git clone --depth 1 https://github.com/ptitSeb/gl4es.git "$GL4ES_DIR"
fi

# ---- Step 2: Apply patches ----
echo "Applying patches..."
cd "$BLENDER_DIR"
for patch in "$PATCHES_DIR"/*.patch; do
	echo "  Applying $(basename $patch)..."
	git apply "$patch" 2>/dev/null || echo "  (already applied or skipped)"
done
cd "$PROJECT_DIR"

# ---- Step 3: Build GL4ES ----
echo "=== Building GL4ES ==="
mkdir -p "$GL4ES_DIR/build"
cd "$GL4ES_DIR/build"
cmake .. \
	-DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
	-DANDROID_ABI="$ARCH" \
	-DANDROID_PLATFORM="$ANDROID_API" \
	-DCMAKE_INSTALL_PREFIX="$GL4ES_DIR/install" \
	-DSTATICLIB=ON \
	-DDEFAULT_ES=2 \
	-DNO_INIT_CONSTRUCTOR=ON \
	-G Ninja
ninja -j"$JOBS"
ninja install
cd "$PROJECT_DIR"

# ---- Step 4: Build Blender .so ----
echo "=== Building Blender .so ==="
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake "$BLENDER_DIR" \
	-DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
	-DANDROID_ABI="$ARCH" \
	-DANDROID_PLATFORM="$ANDROID_API" \
	-DCMAKE_INSTALL_PREFIX="$OUTPUT_DIR/blender279-data" \
	-DWITH_BLENDER=ON \
	-DWITH_PLAYER=OFF \
	-DWITH_CYCLES=ON \
	-DWITH_GAMEENGINE=OFF \
	-DWITH_COMPOSITOR=OFF \
	-DWITH_FREESTYLE=OFF \
	-DWITH_OPENCOLORIO=OFF \
	-DWITH_OPENVDB=OFF \
	-DWITH_CODEC_FFMPEG=OFF \
	-DWITH_CODEC_SNDFILE=OFF \
	-DWITH_OPENAL=OFF \
	-DWITH_SDL=ON \
	-DWITH_GL_EGL=ON \
	-DWITH_SYSTEM_GLES=ON \
	-DWITH_PYTHON=ON \
	-DWITH_INTERNATIONAL=OFF \
	-DWITH_INSTALL_PORTABLE=ON \
	-DWITH_ANDROID_MODULE=ON \
	-DOPENGLES_LIBRARY="$GL4ES_DIR/install/lib/libGLESv2.so" \
	-DOPENGLES_EGL_LIBRARY="$GL4ES_DIR/install/lib/libEGL.so" \
	-DCMAKE_C_FLAGS="-fPIC -static-libgcc -Os" \
	-DCMAKE_CXX_FLAGS="-fPIC -static-libstdc++ -Os" \
	-DCMAKE_EXE_LINKER_FLAGS="-static-libgcc -static-libstdc++ -Wl,-z,max-page-size=4096" \
	-DCMAKE_SHARED_LINKER_FLAGS="-static-libgcc -static-libstdc++ -Wl,-z,max-page-size=4096" \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=BOTH \
	-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=BOTH \
	-DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
	-G Ninja

echo "Building libblender.so..."
ninja -j"$JOBS" blender

echo "=== Build complete ==="

# ---- Step 5: Collect artifacts ----
mkdir -p "$OUTPUT_DIR"

# Blender .so
if [ -f "$BUILD_DIR/bin/libblender.so" ]; then
	cp "$BUILD_DIR/bin/libblender.so" "$OUTPUT_DIR/"
elif [ -f "$BUILD_DIR/lib/libblender.so" ]; then
	cp "$BUILD_DIR/lib/libblender.so" "$OUTPUT_DIR/"
else
	echo "WARNING: libblender.so not found!"
	find "$BUILD_DIR" -name "*.so" -o -name "blender" -type f 2>/dev/null | head -10
fi

# GL4ES .so files
cp "$GL4ES_DIR/install/lib/libGL.so" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$GL4ES_DIR/install/lib/libEGL.so" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$GL4ES_DIR/install/lib/libGLESv2.so" "$OUTPUT_DIR/" 2>/dev/null || true

# Blender data files (scripts, etc)
cmake --install "$BUILD_DIR" --prefix "$OUTPUT_DIR/blender-data" 2>/dev/null || true

echo "=== Artifacts in: $OUTPUT_DIR ==="
ls -lh "$OUTPUT_DIR/"
