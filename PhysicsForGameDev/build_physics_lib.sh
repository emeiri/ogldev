#!/bin/bash

TARGET="../../Lib/libphysics.a"

#if [[ -e "$TARGET" && $(($(date +%s) - $(stat -c %Y "$TARGET"))) -le 3600 ]]; then
	#echo "No need to rebuild library"
	#exit 0
#fi

ROOTDIR="../.."
source $ROOTDIR/build_base.sh

$CC $OGL_CPPFLAGS -I../Include -c *.cpp 

ar rcs $TARGET *.o
