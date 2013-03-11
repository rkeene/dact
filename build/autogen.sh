#! /bin/bash

# Define list of tools copied in and files generated
tools=(config.sub config.guess install-sh)
files=(configure aclocal.m4 config.h.in libdact.vers)

# Start in the source directory
cd "$(dirname "$(which "$0")")/.." || exit 1

# Verify that we are operating in the correct directory
if [ ! -e 'configure.ac' ]; then
	echo "ERROR: Unable to find appropriate directory to build autoconf in"

	exit 1
fi

# Clean-up before starting
rm -f "${tools[@]}" "${files[@]}"
rm -rf build/aclocal

# If all we were asked to do was clean, we are done
if [ "$1" = 'clean' -o "$1" = 'distclean' ]; then
	exit 0
fi

# Create configure script
## Create aclocal.m4
mkdir build/aclocal

### Download third-party autoconf macros
wget -o /dev/null -O build/aclocal/shobj.m4 http://rkeene.org/devel/autoconf/shobj.m4
wget -o /dev/null -O build/aclocal/versionscript.m4 http://rkeene.org/devel/autoconf/versionscript.m4

### Create aclocal.m4 with all appropriate contents
aclocal --warnings=none -I "$(pwd)/build/aclocal" --install

## Create configure script itself
autoconf

## Create config.h
autoheader

## Remove cache directory created by "autoconf"
rm -rf autom4te.cache

# Copy in all tools required by configure script
for dir in /usr/share/automake-*; do
	all_found='1'
	for tool in "${tools[@]}"; do
		if [ ! -e "${dir}/${tool}" ]; then
			all_found='0'
		fi
	done

	if [ "${all_found}" = '1' ]; then
		for tool in "${tools[@]}"; do
			cp "${dir}/${tool}" .
		done

		break
	fi
done

# Create "libdact.vers" (GNU ld version script) from "libdact.syms.in" which
# is a list of public symbols
rm -f libdact.vers
make -f Makefile.in libdact.vers

# Terminate
exit 0
