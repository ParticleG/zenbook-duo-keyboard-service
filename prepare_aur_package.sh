#!/bin/bash

set -e

# Define variables
PKG_NAME="zenbook-duo-keyboard-service"
PKG_VERSION="1.0.0"
# 使用脚本所在目录作为项目路径，而非硬编码的用户路径
BASE_DIR="$(dirname "$(readlink -f "$0")")"
SOURCE_DIR="$BASE_DIR/$PKG_NAME"
TEMP_DIR="/tmp/$PKG_NAME-build"

# Create temporary directory
echo "Creating temporary build directory..."
rm -rf "$TEMP_DIR"
mkdir -p "$TEMP_DIR/$PKG_NAME"

# Copy necessary source files to temporary directory
echo "Copying source files..."
cp -r "$BASE_DIR/src" "$TEMP_DIR/$PKG_NAME/"
cp -r "$BASE_DIR/bin" "$TEMP_DIR/$PKG_NAME/"
cp -r "$BASE_DIR/etc" "$TEMP_DIR/$PKG_NAME/"
cp "$BASE_DIR/README.md" "$TEMP_DIR/$PKG_NAME/"
cp "$BASE_DIR/LICENSE" "$TEMP_DIR/$PKG_NAME/"

# Create source package
echo "Creating source package..."
cd "$TEMP_DIR"
# Create source package in temp directory instead of project root
tar -czf "$TEMP_DIR/$PKG_NAME-$PKG_VERSION.tar.gz" "$PKG_NAME"

# Calculate checksum
echo "Calculating source package checksum..."
SHA256SUM=$(sha256sum "$TEMP_DIR/$PKG_NAME-$PKG_VERSION.tar.gz" | awk '{print $1}')
echo "Calculated checksum: $SHA256SUM"

# Prepare AUR package directory first
echo "Preparing AUR package directory..."
rm -rf "$BASE_DIR/aur"
mkdir -p "$BASE_DIR/aur"

# Copy files to AUR directory first, without modifying original files
echo "Copying files to AUR directory..."
cp "$BASE_DIR/PKGBUILD" "$BASE_DIR/aur/"
cp "$BASE_DIR/$PKG_NAME.install" "$BASE_DIR/aur/"
# Move source package to AUR directory
mv "$TEMP_DIR/$PKG_NAME-$PKG_VERSION.tar.gz" "$BASE_DIR/aur/"

# Now update PKGBUILD in AUR directory
echo "Updating checksum in AUR directory's PKGBUILD..."
if grep -q "sha256sums=" "$BASE_DIR/aur/PKGBUILD"; then
    # Use generic regex to replace any format of sha256sums line
    sed -i -E "s/sha256sums=\(['\"]?[^)]*['\"]?\)/sha256sums=('$SHA256SUM')/" "$BASE_DIR/aur/PKGBUILD"
    echo "Replaced sha256sums in AUR directory's PKGBUILD with new value"
else
    echo "Warning: Could not find sha256sums line in PKGBUILD, adding manually"
    echo "sha256sums=('$SHA256SUM')" >> "$BASE_DIR/aur/PKGBUILD"
fi

# Verify successful replacement
if grep -q "$SHA256SUM" "$BASE_DIR/aur/PKGBUILD"; then
    echo "Checksum successfully updated in AUR directory's PKGBUILD"
else
    echo "Error: Could not update checksum in AUR directory's PKGBUILD"
    exit 1
fi

# Generate .SRCINFO file
echo "Generating .SRCINFO file..."
cd "$BASE_DIR/aur"
makepkg --printsrcinfo > .SRCINFO

echo "AUR package preparation complete!"
echo "Package directory: $BASE_DIR/aur"
echo "Source package: $BASE_DIR/$PKG_NAME-$PKG_VERSION.tar.gz"
echo ""
echo "You can now test the package build with:"
echo "cd $BASE_DIR/aur && makepkg -si"
echo ""
echo "After successful build, you can submit to AUR:"
echo "cd $BASE_DIR/aur"
echo "git init"
echo "git add PKGBUILD .SRCINFO"
echo "git commit -m \"Initial commit\""
echo "git remote add origin ssh://aur@aur.archlinux.org/$PKG_NAME.git"
echo "git push -u origin master"

# Clean up temporary directory
rm -rf "$TEMP_DIR"
