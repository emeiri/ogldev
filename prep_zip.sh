#!/bin/bash

rm -rf ogldev-source
rm -f ogldev-source.zip
git clone https://github.com/emeiri/ogldev.git ogldev-source
rm -rf ogldev-source/.git
rm -rf ogldev-source/web
zip -r ogldev-source ogldev-source
