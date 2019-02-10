#!/usr/bin/env bash


for IMGNAME in ./*.svg; do
    echo $IMGNAME
    # Process $i
    #inkscape $IMGNAME.svg --export-pdf=$IMGNAME.pdf &
    BASENAME="${IMGNAME%.*}"
    echo $BASENAME
    
    cairosvg $IMGNAME -o $BASENAME.pdf

    inkscape -z --export-dpi 300 -e $BASENAME.png  $BASENAME.svg
    #convert $BASENAME.png $BASENAME.gif
    #convert $BASENAME.gif $BASENAME.png
    
    echo ==================================
done
