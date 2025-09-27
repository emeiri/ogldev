#!/bin/bash

source ../build_base.sh

LDFLAGS=`pkg-config --libs glew`
LDFLAGS="$LDFLAGS -lglut"

$CC tutorial03.cpp $OGL_CPPFLAGS $LDFLAGS -o tutorial03
