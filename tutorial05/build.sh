#!/bin/bash

source ../build_base.sh

LDFLAGS=`pkg-config --libs glew`
LDFLAGS="$LDFLAGS -lglut"

$CC tutorial05.cpp ../Common/ogldev_util.cpp $OGL_CPPFLAGS $LDFLAGS -o tutorial05
