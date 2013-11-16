#!/bin/bash

rm -rf ogldev-source
rm -f ogldev-source.zip
git clone https://etay.meiri@code.google.com/p/ogldev/ ogldev-source
rm -rf ogldev-source/.git
rm -ff ogldev-source/tutorial43
zip -r ogldev-source ogldev-source

echo Done - no tutorial 43!!!
