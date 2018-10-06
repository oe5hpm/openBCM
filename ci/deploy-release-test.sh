#!/bin/sh

ARCHLIST=`ls | grep -i out-`

mkdir -p out
for i in $ARCHLIST; do
	echo prepare $i ...
	if echo $i | grep -c win32 >/dev/null; then cp $i/* out/; continue; fi
	if echo $i | grep -c DOS >/dev/null; then cp $i/* out/; continue; fi
	ARCH=`echo $i | cut -f 2 -d '-'`
	cp $i/bcm-$ARCH out/bcm-$ARCH
done

ls -l out/*

