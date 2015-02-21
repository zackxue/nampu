#! /bin/sh
set -x

touch AUTHORS NEWS README ChangeLog autoscan.log

aclocal

libtoolize --force -f -c

autoheader

automake --foreign --add-missing --copy

autoconf

./configure --prefix=$NMP_LIB_INSTALL
