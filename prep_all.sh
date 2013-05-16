#!/bin/bash

for dir in tutorial*
do
	if [ -d $dir ]; then
		./prep.sh $dir
	fi
done
