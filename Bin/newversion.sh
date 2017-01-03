#!/bin/sh
#
# Copyright 1992, 1993 by Pix Technologies Corp.
#

version=1.6.0

if [ $# = 1 ]; then
	TARGET=$1
else
	TARGET=unknown
fi
bfile=.buildnumber
if [ ! -r "${bfile}" ]; then
	echo 0 > ${bfile}
fi
touch ${bfile}
b=$(cat ${bfile}) u=${USER-root} d=$(pwd) h=$(hostname) t=$(date)

FILE=version.c

echo "char *version1 = \"Xtank Release ${version}\";" > ${FILE}
echo "char *version2 = \"Build #${b}: (${TARGET}) ${t}\";" >> ${FILE}
echo "char *version3 = \"${u}@${h}:${d}\";" >> ${FILE}
echo $(expr ${b} + 1) > ${bfile}
