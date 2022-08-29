#!/usr/bin/env bash

# Run as part of this command:
# find . -maxdepth 1 -iname "*.svg" -exec bash ./rasterize.sh {} \;
#  find . -maxdepth 1 -iname "*.svg" -exec bash ./rasterize_all.sh {} \;

IMGNAME=${1%.*}
echo $IMGNAME

#inkscape $IMGNAME.svg --export-pdf=$IMGNAME.pdf &
#cairosvg $IMGNAME.svg -o $IMGNAME.pdf

inkscape -z --export-dpi 1200 -e $IMGNAME.png  $IMGNAME.svg
convert $IMGNAME.png $IMGNAME.gif
convert $IMGNAME.gif $IMGNAME.png
convert $IMGNAME.png  -alpha off -fill '#FFFFFE' -opaque '#FFFFFF' -alpha on -background white -flatten $IMGNAME.png
rm -f $IMGNAME.gif 
