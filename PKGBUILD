pkgname=instashare
pkgver=1
pkgrel=1
pkgdesc='image sharing application'
arch=('i686' 'x86_64')
url='https://github.com/xil-se/QtInstaShare'
license=('Custom')
depends=('qt5-base')
makedepends=('qt5-base')
source=()
install="ArchInstaShare.install"
pkgver() {
  cd "$startdir"
  local ver="$(git describe --tags)"
  ver="${ver#v}"
  echo "${ver//-/.}"
}

build() {
  cd "$startdir"
  qmake
  make
}

package() {
  cd "$startdir"
  install -Dm0555 "$startdir"/bin/InstaShare "$pkgdir/usr/bin/$pkgname"
}
