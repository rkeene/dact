#!/bin/sh

cd /home/rkeene/devel/compression/

VER=`/usr/bin/grep "DACT_VER_" dact.h | /bin/cut -f 3 -d " " | /usr/bin/tr "\n" . | /bin/cut -f 1-3 -d "."`
DIRECT=`pwd`
cd ../
echo "Making archive `pwd`/dact-$VER.tar.gz"
cd $DIRECT

VER_MAJ=`echo $VER | cut -f 1 -d .`
VER_MIN=`echo $VER | cut -f 2 -d .`
VER_REV=`echo $VER | cut -f 3 -d .`

mkdir -p ../dact-$VER/Docs
cp * ../dact-$VER 2>/dev/null >/dev/null
cp Docs/* ../dact-$VER/Docs/ 2>/dev/null >/dev/null
cd ../dact-$VER

rm -f makearch.sh .* *.dct *.bak *.swp *.exe makesnapshot.sh *.o joe* bob* f 2>/dev/null >/dev/null

for i in *.[ch]; do 
	grep -v ^// $i > f
	mv f $i
done

./configure 2>&1 >/dev/null
/usr/bin/make distclean >/dev/null 2>/dev/null
/usr/bin/make Makefile.dep 2>/dev/null >/dev/null
/usr/bin/sed s/"DACT_VERSION"/$VER/g README > f
/bin/mv f README
/usr/bin/md5sum * 2>/dev/null > MD5SUM
/usr/bin/file * 2>/dev/null > FILETYPE
printf "%03i%03i%03i\n" $VER_MAJ $VER_MIN $VER_REV > VERSION
cd ..
/bin/tar -zhcf dact-$VER.tar.gz dact-$VER/*
/bin/rm -rf dact-$VER
