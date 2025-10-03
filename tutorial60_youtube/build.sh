#!/bin/bash

source ../build_base.sh

build_demolition

$CC tutorial60.cpp -I../DemoLITION/Framework/Include/ $OGL_CPPFLAGS ../Lib/libdemolition.a  $OGL_LDFLAGS -o tutorial60
