#! /bin/sh

if echo " $* " | grep ' debug ' >/dev/null; then
	DEBUG="--enable-debug"
else
	DEBUG="--disable-debug"
fi
if [ ! -x configure ]; then cd ../; fi
WIN32="${HOME}/root/windows-i386"
CFLAGS="-I${WIN32}/include"
CPPFLAGS="${CFLAGS}"
LDFLAGS="-L${WIN32}/lib"
DATE="`date +%Y%m%d%H%M`"
CROSS=i586-mingw32msvc
if [ ! -z "${CROSS}" ]; then
	CROSSCMD="${CROSS}-"
fi
export CFLAGS LDFLAGS CPPFLAGS
make distclean
./configure --host=${CROSS} --prefix=c:/dact/ ${DEBUG} && \
make && \
${CROSSCMD}strip dact.exe
if echo " $* " | grep ' web ' >/dev/null; then
	cp dact.exe "/web/rkeene/files/oss/dact/snapshot/binary/dact-${DATE}.exe"
fi
