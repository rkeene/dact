#!/bin/bash

cd /home/rkeene/devel/compression

VER=`/usr/bin/grep "DACT_VER_" dact.h | /bin/cut -f 3 -d " " | /usr/bin/tr "\n" . | /bin/cut -f 1-3 -d "."`

./makearch.sh 2>/dev/null >/dev/null

cp ../dact-$VER.tar.gz /web/rkeene/devel/dact-snapshot.tar.gz
