#!/bin/bash

source ../build_base.sh

build_ogldev

$CC *.cpp $OGL_CPPFLAGS ../Lib/libogldev.a $OGL_LDFLAGS -o tutorial40
