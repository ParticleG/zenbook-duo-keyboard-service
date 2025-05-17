# Maintainer: ParticleG <particle_g@outlook.com>

pkgname=zenbook-duo-keyboard-service
pkgver=1.0.0
pkgrel=1
pkgdesc="Keyboard service for ASUS Zenbook Duo"
arch=('x86_64')
url="https://github.com/ParticleG/zenbook-duo-keyboard-service"
license=('GPL')
depends=('libusb')
makedepends=('gcc')
source=("$pkgname-$pkgver.tar.gz")
sha256sums=('')
install=$pkgname.install

prepare() {
  cd "$srcdir/$pkgname"
  mkdir -p build
}

build() {
  cd "$srcdir/$pkgname"
  gcc -Wall -o build/service src/service.c -lusb-1.0
  cp -r bin/* build/
}

package() {
  cd "$srcdir/$pkgname"
  
  # Create directories
  install -dm755 "$pkgdir/usr/bin"
  install -dm755 "$pkgdir/etc/keyboard-service"
  install -dm755 "$pkgdir/usr/lib/systemd/system"
  install -dm755 "$pkgdir/var/lib/keyboard-service"
  
  # Install built artifacts
  install -m755 "build/service" "$pkgdir/usr/bin/keyboard-service"
  install -m755 "build/control.sh" "$pkgdir/usr/bin/keyboard-control"
  install -m644 "build/zenbook-duo-keyboard.service" "$pkgdir/usr/lib/systemd/system/zenbook-duo-keyboard.service"
  
  # Install configuration file
  install -m644 "etc/hypr_monitor.conf" "$pkgdir/etc/keyboard-service/hypr_monitor.conf"
  
  # Install documentation
  install -dm755 "$pkgdir/usr/share/doc/$pkgname"
  install -m644 "README.md" "$pkgdir/usr/share/doc/$pkgname/"
  install -m644 "LICENSE" "$pkgdir/usr/share/doc/$pkgname/"
}
