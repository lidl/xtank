#!/bin/sh -
#
# Copyright 1992, 1993 by Pix Technologies Corp.
#
# $Id$
#
if [ $# = 1 ]; then
    TARGET=$1
else
    TARGET=unknown
fi
if [ ! -r .version ]; then
        /bin/echo 0 > .version
fi
touch .version
v=`cat .version` u=${USER-root} d=`pwd` h=`hostname` t=`date`

FILE=version.c

/bin/echo "char *version1 = \"Xtank Pix.COM Release 1.6.0\";" > ${FILE}
/bin/echo "char *version2 = \"Build #${v}: (${TARGET}) ${t}\";" >> ${FILE}
/bin/echo "char *version3 = \"${u}@${h}:${d}\";" >> ${FILE}
/bin/echo `expr ${v} + 1` > .version
