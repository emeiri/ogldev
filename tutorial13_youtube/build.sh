#!/bin/bash

source ../build_base.sh

LDFLAGS=`pkg-config --libs glew`
LDFLAGS="$LDFLAGS -lglut"

$CC tutorial13.cpp ../Common/ogldev_util.cpp $OGL_CPPFLAGS $LDFLAGS -o tutorial13
