#! /bin/sh

if [ ! -x configure ]; then cd ../; fi
CFLAGS='-Iwork/include'
CPPFLAGS="${CFLAGS}"
LDFLAGS='-Lwork/lib'
DATE="`date +%Y%m%d%H%M`"
CROSS=i586-mingw32msvc
if [ ! -z "${CROSS}" ]; then
	CROSSCMD="${CROSS}-"
fi
export CFLAGS LDFLAGS CPPFLAGS
./configure --host=${CROSS} --prefix=c:/dact/ --disable-debug && \
make && \
${CROSSCMD}strip dact.exe && \
cp dact.exe "/web/rkeene/files/oss/dact/snapshot/binary/dact-${DATE}.exe"
