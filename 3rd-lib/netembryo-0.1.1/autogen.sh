#! /bin/sh
set -x

touch AUTHORS NEWS README ChangeLog

aclocal

libtoolize --force -f -c

autoheader

automake -a #--foreign --add-missing --copy

autoreconf -i

./configure --prefix=$NMP_LIB_INSTALL
