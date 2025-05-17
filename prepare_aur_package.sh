#!/bin/bash

set -e

# Define variables
PKG_NAME="zenbook-duo-keyboard-service"
PKG_VERSION="1.0.0"
BASE_DIR="/home/particleg/projects/zenbook-duo-keyboard-service"
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
tar -czf "$BASE_DIR/$PKG_NAME-$PKG_VERSION.tar.gz" "$PKG_NAME"

# Calculate checksum and update PKGBUILD
echo "Updating checksum in PKGBUILD..."
SHA256SUM=$(sha256sum "$BASE_DIR/$PKG_NAME-$PKG_VERSION.tar.gz" | awk '{print $1}')
sed -i "s/sha256sums=('SKIP')/sha256sums=('$SHA256SUM')/" "$BASE_DIR/PKGBUILD"

# Prepare AUR package directory
echo "Preparing AUR package directory..."
rm -rf "$BASE_DIR/aur"
mkdir -p "$BASE_DIR/aur"
cp "$BASE_DIR/PKGBUILD" "$BASE_DIR/aur/"
cp "$BASE_DIR/$PKG_NAME.install" "$BASE_DIR/aur/"
cp "$BASE_DIR/$PKG_NAME-$PKG_VERSION.tar.gz" "$BASE_DIR/aur/"

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
