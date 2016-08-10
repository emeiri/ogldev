#!/bin/bash

for filename in tutorial*; do
	cd $filename
	make
	cd ..
done
