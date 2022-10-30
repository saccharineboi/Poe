#!/bin/sh
# Convert all TGA textures to PNG textures

cnt=0
for tga_file in *.tga; do
    cnt=$((cnt + 1))
done

i=0
for tga_file in *.tga; do
    printf "Converting file %s (%d out of %d)\n" $tga_file $((i+1)) $cnt
    convert -quality 99 -flip $tga_file ${tga_file%.*}.png
    i=$((i + 1))
done

echo "Finished!"
