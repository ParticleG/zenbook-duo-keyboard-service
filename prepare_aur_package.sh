#!/bin/bash

set -e

# 定义变量
PKG_NAME="zenbook-duo-keyboard-service"
PKG_VERSION="1.0.0"
BASE_DIR="/home/particleg/projects/zenbook-duo-keyboard-service"
SOURCE_DIR="$BASE_DIR/$PKG_NAME"
TEMP_DIR="/tmp/$PKG_NAME-build"

# 创建临时目录
echo "创建临时构建目录..."
rm -rf "$TEMP_DIR"
mkdir -p "$TEMP_DIR/$PKG_NAME"

# 复制必要的源文件到临时目录
echo "复制源文件..."
cp -r "$BASE_DIR/src" "$TEMP_DIR/$PKG_NAME/"
cp -r "$BASE_DIR/bin" "$TEMP_DIR/$PKG_NAME/"
cp -r "$BASE_DIR/etc" "$TEMP_DIR/$PKG_NAME/"
cp "$BASE_DIR/README.md" "$TEMP_DIR/$PKG_NAME/"
cp "$BASE_DIR/LICENSE" "$TEMP_DIR/$PKG_NAME/"

# 创建源码包
echo "创建源码包..."
cd "$TEMP_DIR"
tar -czf "$BASE_DIR/$PKG_NAME-$PKG_VERSION.tar.gz" "$PKG_NAME"

# 计算校验和并更新PKGBUILD
echo "更新PKGBUILD中的校验和..."
SHA256SUM=$(sha256sum "$BASE_DIR/$PKG_NAME-$PKG_VERSION.tar.gz" | awk '{print $1}')
sed -i "s/sha256sums=('SKIP')/sha256sums=('$SHA256SUM')/" "$BASE_DIR/PKGBUILD"

# 准备AUR包目录
echo "准备AUR包目录..."
rm -rf "$BASE_DIR/aur"
mkdir -p "$BASE_DIR/aur"
cp "$BASE_DIR/PKGBUILD" "$BASE_DIR/aur/"
cp "$BASE_DIR/$PKG_NAME.install" "$BASE_DIR/aur/"
cp "$BASE_DIR/$PKG_NAME-$PKG_VERSION.tar.gz" "$BASE_DIR/aur/"

# 生成.SRCINFO文件
echo "生成.SRCINFO文件..."
cd "$BASE_DIR/aur"
makepkg --printsrcinfo > .SRCINFO

echo "AUR包准备完成！"
echo "包目录: $BASE_DIR/aur"
echo "源码包: $BASE_DIR/$PKG_NAME-$PKG_VERSION.tar.gz"
echo ""
echo "现在您可以通过以下方式测试包的构建:"
echo "cd $BASE_DIR/aur && makepkg -si"
echo ""
echo "成功构建后，您可以提交到AUR:"
echo "cd $BASE_DIR/aur"
echo "git init"
echo "git add PKGBUILD .SRCINFO"
echo "git commit -m \"Initial commit\""
echo "git remote add origin ssh://aur@aur.archlinux.org/$PKG_NAME.git"
echo "git push -u origin master"

# 清理临时目录
rm -rf "$TEMP_DIR"
