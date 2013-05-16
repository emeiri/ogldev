#!/bin/bash

rm -rf ogldev-source
rm -f ogldev-source.zip
svn export http://ogldev.googlecode.com/svn/trunk/ ogldev-source
zip -r ogldev-source ogldev-source
