#!/bin/bash

CC=g++
LDFLAGS=`pkg-config --libs glew`
LDFLAGS="$LDFLAGS -lglut"

$CC freeglut_multiple_windows.cpp  $LDFLAGS -o freeglut_multiple_windows
