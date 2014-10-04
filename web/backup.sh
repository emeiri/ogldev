#!/bin/bash

rm tutorials.tar tutorials.tar.bz2 2> /dev/null
tar cvf tutorials.tar .git
bzip2 tutorials.tar
