#!/bin/bash

source ../build_base.sh

PWD=`pwd`
PROJ_NAME=`basename $PWD`
PROJ_SRC=$PROJ_NAME.cpp
PROJ_OBJ=$PROJ_NAME.o
$CC $CPPFLAGS $PROJ_SRC -o $PROJ_OBJ
$CC $PROJ_OBJ -o $PROJ_NAME $LDFLAGS
