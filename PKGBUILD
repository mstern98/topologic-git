#Maintainer: Matthew Stern <msstern98 gmail com>
pkgname=Topologic
pkgver=1.0.2
pkgrel=1
pkgdesc='Library to create dyanimic context free or switching automata'
arch=('any')
url="https://github.com/mstern98/Topologic"
license=('MIT')
depends=(bison flex)


source=("${pkgname}-${pkgver}.tar.gz::https://github.com/mstern98/Topologic/archive/v${pkgver}.tar.gz")
sha256sums=('0d24c93771b1df69fc6e3cd19e1a350591f3fe456d11b1a2a7d9b19306a00f7b')

build () 
{
    cd "$srcdir/${pkgname}-${pkgver}"
    echo "Starting Build..."
    make
}

package ()
{
    mkdir -p "$pkgdir/usr/include"
    mkdir -p "$pkgdir/usr/include/topologic"
    mkdir -p "$pkgdir/usr/lib"
    
    cp "$srcdir/${pkgname}-${pkgver}/include/AVL.h" "$pkgdir/usr/include/topologic/"
    cp "$srcdir/${pkgname}-${pkgver}/include/context.h" "$pkgdir/usr/include/topologic/"
    cp "$srcdir/${pkgname}-${pkgver}/include/edge.h" "$pkgdir/usr/include/topologic/"
    cp "$srcdir/${pkgname}-${pkgver}/include/graph.h" "$pkgdir/usr/include/topologic/"
    cp "$srcdir/${pkgname}-${pkgver}/include/request.h" "$pkgdir/usr/include/topologic/"
    cp "$srcdir/${pkgname}-${pkgver}/include/stack.h" "$pkgdir/usr/include/topologic/"
    cp "$srcdir/${pkgname}-${pkgver}/include/topologic.h" "$pkgdir/usr/include/topologic/"
    cp "$srcdir/${pkgname}-${pkgver}/include/vertex.h" "$pkgdir/usr/include/topologic/"
    cp "$srcdir/${pkgname}-${pkgver}/include/header.h" "$pkgdir/usr/include/topologic/"
    cp "$srcdir/${pkgname}-${pkgver}/libtopologic.a" "$pkgdir/usr/lib/"
}
