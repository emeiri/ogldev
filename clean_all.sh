#!/bin/bash

for filename in tutorial*; do
	cd $filename
	make clean
	cd ..
done
