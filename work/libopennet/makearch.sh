#!/bin/sh

APPL=libopennet


VER=`/usr/bin/grep "^\#define VERSION" conf.h | /bin/cut -f 3 -d " " | /usr/bin/tr "\n" . | /bin/cut -f 1-3 -d "." | sed s/"\""/""/g`
DIRECT=`pwd`
cd ../
echo "Making archive `pwd`/$APPL-$VER.tar.gz"
/bin/mkdir $APPL-$VER
cd $DIRECT

/bin/cp * ../$APPL-$VER/ 2> /dev/null

cd ../$APPL-$VER/

/usr/bin/sed s/"__VERSION__"/$VER/g README > f
/bin/mv f README
/bin/rm makearch.sh
make distclean >/dev/null

cd ..
/bin/tar -zhcf $APPL-$VER.tar.gz $APPL-$VER/*
/bin/rm -rf $APPL-$VER
cd $DIRECT


