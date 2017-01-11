#!/bin/sh
# /*-
# * Copyright (c) 1992,1993 Josh Osborne
# * Copyright (c) 1992,1993,1999,2017 Kurt Lidl
# * All rights reserved.
# *
# * Redistribution and use in source and binary forms, with or without
# * modification, are permitted provided that the following conditions
# * are met:
# * 1. Redistributions of source code must retain the above copyright
# *    notice, this list of conditions and the following disclaimer.
# * 2. Redistributions in binary form must reproduce the above copyright
# *    notice, this list of conditions and the following disclaimer in the
# *    documentation and/or other materials provided with the distribution.
# *
# * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# * SUCH DAMAGE.
# */

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
