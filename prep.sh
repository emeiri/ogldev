#!/bin/bash

if [ $# -ne 1 ] 
then
	echo "Usage: " $0 " <DirName>"
fi

cd $1

make CONF=Debug clean
make CONF=Release clean

cd .. 

filename=$1.tar.gz
tar czvf $filename --exclude=.svn $1
