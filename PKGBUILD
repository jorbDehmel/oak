# Maintainer: Jordan Dehmel <jdehmel@outlook.com>

pkgname="oak-git"
pkgver="0.0.10"
pkgrel="1"
pkgdesc="The Oak programming language and Acorn compiler"
arch=("x86_64")
depends=("clang" "make" "git")
license=("GPLv3")
source=("git+https://github.com/jorbDehmel/oak.git")
sha512sums=("SKIP")
provides=("oak" "oak-git")

package() {
	make -C oak install
}
