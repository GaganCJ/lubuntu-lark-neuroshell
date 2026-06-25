#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# --- Configuration ---
PACKAGE_NAME="lubuntu-lark-neuroshell"
ARCH="amd64"

# Dynamic Versioning using Git. This creates a version like "1.0.0-5-git123abc".
# It requires you to have at least one git tag (e.g., `git tag -a v1.0.0 -m "Version 1.0.0"`).
VERSION=$(git describe --tags --long --dirty | sed 's/^v//' | sed 's/-g/-git/')

STAGING_DIR="${PACKAGE_NAME}_${VERSION}_${ARCH}"

echo "--- Starting Lubuntu Lark Build and Packaging Process ---"

# --- 1. Cleanup ---
echo "[1/7] Cleaning up previous build artifacts..."
rm -rf build
rm -rf "$STAGING_DIR"
rm -f "${STAGING_DIR}.deb"
echo "Cleanup complete."

# --- 2. Compilation ---
echo "[2/7] Compiling C++ plugins via CMake..."
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
cd ..
echo "Compilation complete."

# --- 3. Staging Area Setup ---
echo "[3/7] Creating Debian staging directory structure: $STAGING_DIR"
mkdir -p "$STAGING_DIR/DEBIAN"
mkdir -p "$STAGING_DIR/usr/local/bin"
mkdir -p "$STAGING_DIR/etc/xdg/autostart"
mkdir -p "$STAGING_DIR/usr/lib/x86_64-linux-gnu/lxqt/plugins/panel"
mkdir -p "$STAGING_DIR/usr/lib/x86_64-linux-gnu/lxqt-config-mods"
mkdir -p "$STAGING_DIR/usr/share/applications"
echo "Staging directories created."

# --- 4. Copying Files ---
echo "[4/7] Copying files to staging area..."

# Copy compiled .so files from the build directory
echo "  - Copying shared object (.so) files..."
cp build/liblxqt_ai_widget.so "$STAGING_DIR/usr/lib/x86_64-linux-gnu/lxqt/plugins/panel/"

# Copy DEBIAN control files
echo "  - Generating DEBIAN control file with dynamic version..."
cat > "$STAGING_DIR/DEBIAN/control" <<EOF
Package: ${PACKAGE_NAME}
Version: ${VERSION}
Architecture: ${ARCH}
Maintainer: Gagan C J <gaganjchandra2379@gmail.com>
Depends: lxqt-panel, lxqt-config, lxqt-session, libqt6widgets6, qt6-webengine-dev
Description: Lubuntu Lark: Cloud-Hybrid Dual-Provider AI Applet
 Lubuntu Lark is an optimized, privacy-first, cloud-hybrid AI taskbar widget
 built on top of the LXQt (Qt6) Desktop Environment. This applet streams
 Google Gemini and Microsoft Copilot with dual persistent cookie sessions and
 low-footprint Chromium optimizations.
EOF

# Copy other DEBIAN files
cp preinst "$STAGING_DIR/DEBIAN/"
echo "File copying complete."

# --- 5. Set Permissions ---
echo "[5/7] Setting executable permissions..."
chmod 755 "$STAGING_DIR/DEBIAN/preinst"
echo "Permissions set."

# --- 6. Build the Debian Package ---
echo "[6/7] Building the Debian package..."
dpkg-deb --build "$STAGING_DIR"
echo "Package build complete."

# --- 7. Final Cleanup ---
echo "[7/7] Cleaning up staging directory..."
rm -rf "$STAGING_DIR"

echo ""
echo "--- Packaging Successful! ---"
echo "Package created: ${STAGING_DIR}.deb"
