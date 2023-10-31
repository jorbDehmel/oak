# Maintainer: Jordan Dehmel <jdehmel@outlook.com>

pkgname="oak-git"
pkgver="0.2.3"
pkgrel="1"
pkgdesc="The Oak programming language and Acorn compiler"
depends=("clang" "make" "git")
license=("GPLv3")
source=("git+https://github.com/jorbDehmel/oak.git")
sha512sums=("SKIP")
provides=("oak" "oak-git" "acorn")

package() {
	make -C oak install
}
