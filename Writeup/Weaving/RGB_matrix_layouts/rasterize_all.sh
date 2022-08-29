#!/usr/bin/env bash
ME=`echo $0`
parentdir="$(dirname "$ME")"
echo $parentdir
cd "$parentdir"
find . -maxdepth 1 -iname "*.svg" -exec bash ./rasterize.sh {} \;

mv *.png ../png
