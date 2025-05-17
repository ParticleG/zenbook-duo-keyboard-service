#!/bin/bash

set -e

# Define variables
PKG_NAME="zenbook-duo-keyboard-service"
PKG_VERSION="1.0.0"
BASE_DIR="/home/particleg/projects/zenbook-duo-keyboard-service"

# Create package
echo "Creating AUR package..."
cd "$BASE_DIR"
tar -czf "$PKG_NAME-$PKG_VERSION.tar.gz" "$PKG_NAME"

# 计算校验和并更新PKGBUILD
echo "更新PKGBUILD中的校验和..."
SHA256SUM=$(sha256sum "$PKG_NAME-$PKG_VERSION.tar.gz" | awk '{print $1}')
sed -i "s/sha256sums=('SKIP')/sha256sums=('$SHA256SUM')/" "$PKG_NAME/PKGBUILD"

# 生成.SRCINFO文件
echo "生成.SRCINFO文件..."
cd "$PKG_NAME"
makepkg --printsrcinfo > .SRCINFO

echo "AUR包准备完成！"
echo "包位置: $BASE_DIR/$PKG_NAME"
echo "源码包: $BASE_DIR/$PKG_NAME-$PKG_VERSION.tar.gz"
echo ""
echo "现在您可以通过以下方式测试包的构建:"
echo "cd $BASE_DIR/$PKG_NAME && makepkg -si"
echo ""
echo "成功构建后，您可以提交到AUR:"
echo "git init"
echo "git add PKGBUILD .SRCINFO"
echo "git commit -m \"Initial commit\""
echo "git remote add origin ssh://aur@aur.archlinux.org/$PKG_NAME.git"
echo "git push -u origin master"
