#! /bin/sh

if [ ! -x configure ]; then cd ../; fi
CFLAGS='-Iwork/include'
CPPFLAGS="${CFLAGS}"
LDFLAGS='-Lwork/lib'
export CFLAGS LDFLAGS CPPFLAGS
./configure --host=i586-mingw32msvc
