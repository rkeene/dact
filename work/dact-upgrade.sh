#! /bin/sh

MACHINE=`uname -m | sed s/"i.86"/"i386"/ | dd conv=lcase 2>/dev/null`
OS=`uname -s | dd conv=lcase 2>/dev/null`
FILE="http://www.rkeene.org/devel/dact/precompiled/dact-$MACHINE-$OS.bin"
LFILE="dact-$MACHINE-$OS.bin"
VERSION=http://www.rkeene.org/devel/dact/VERSION

if [ "$1" = "detcomp" ]; then
  echo $LFILE
  exit
fi

if [ "$1" = "define" ]; then
  MCRO=_`echo $2 | tr [a-z]\. [A-Z]_ | sed s/"_C$"/"_H"/`
  echo "#ifndef $MCRO"
  echo "#define $MCRO"
  echo "#include \"dact.h\""
  echo ""
  egrep '^ *(char|int|void|uint32_t|uint16_t|unsigned|signed).* .*(.*).* {$' $2  | sed s/" {$"/';'/
  echo "#endif"
  exit
fi

CURRDIR=`pwd`
DIR=/tmp
while [ -d $DIR ]; do
  DIR=/tmp/dact-$RANDOM
done
mkdir $DIR
if [ ! -d $DIR ]; then
  echo "Could not make directory for temporary storage."
  exit 1
fi
cd $DIR



if which lynx 2>/dev/null >/dev/null; then
  lynx -nostatus -dump $VERSION > VERSION 2>/dev/null
  CMD="lynx -nostatus -dump $FILE > $LFILE"
elif which wget 2>/dev/null >/dev/null; then
  wget -q $VERSION 2>/dev/null >/dev/null
  CMD="wget -q $FILE"
else
  echo "No appropriate way to download."
  echo "The file is at:"
  echo "$FILE"
  cd $CURRDIR
  rmdir $DIR
  exit 1
fi

CURR_VER=`dact -h 2>&1 | head -1 | sed s/\ version// | cut -f 2 -d \  | cut -f 1 -d -`
VER_MAJ=`cut VERSION -c 1-3|sed s/"^00\?"//`
VER_MIN=`cut VERSION -c 4-6|sed s/"^00\?"//`
VER_REV=`cut VERSION -c 7-9|sed s/"^00\?"//`
NEW_VER=`echo "$VER_MAJ.$VER_MIN.$VER_REV"`
if [ "$NEW_VER" = "$CURR_VER" ]; then
  echo "Would download same version."
  rm VERSION
  cd $CURRDIR
  rmdir $DIR
  exit 1
fi
echo "Do you really want to upgrade from DACT $CURR_VER to DACT $NEW_VER ?"
read YN
YN=`echo $YN | dd conv=ucase bs=1 count=1 2>/dev/null`
if [ ! "$YN" = "Y" ]; then
  rm VERSION
  cd $CURRDIR
  rmdir $DIR
  exit
fi

eval $CMD

if [ -f $LFILE ]; then
 if head -2 $LFILE | grep "Not Found" 2>/dev/null >/dev/null; then
   rm $LFILE
 fi 
fi

if [ ! -f $LFILE ]; then
  echo "Unable to find an appropriate precompiled binary for a $MACHINE running $OS"
  echo "Contact Roy Keene <rkeene@rkeene.org> if you need further assistance."
  echo "The latest dact source can be found at:"
  echo "http://www.rkeene.org/devel/dact.tar.gz"
  rmdir $DIR
  cd $CURRDIR
  exit 1
fi

chmod 0755 $LFILE

ID=`id | cut -f 2 -d = | cut -f 1 -d "("`
if [ $ID = 0 ]; then
  cp $LFILE /usr/local/bin
  cat /usr/local/bin/dact > /usr/local/bin/dact.old 2>/dev/null
  rm -f /usr/local/bin/dact
  ln -s /usr/local/bin/$LFILE /usr/local/bin/dact
  chown root /usr/local/bin/dact.old /usr/local/bin/$LFILE
  chgrp root /usr/local/bin/dact.old /usr/local/bin/$LFILE
  chmod 0755  /usr/local/bin/dact.old /usr/local/bin/$LFILE
else
  mkdir $HOME/.dact/ 2>/dev/null >/dev/null
  cp $LFILE $HOME/.dact/dact.bin
fi

rm $LFILE
rm VERSION
cd $CURRDIR
rmdir $DIR
