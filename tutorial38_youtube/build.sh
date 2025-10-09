#!/bin/bash

source ../build_base.sh

build_ogldev

$CC tutorial38.cpp $OGL_CPPFLAGS ../Lib/libogldev.a $OGL_LDFLAGS -o tutorial39
