#!/bin/sh
# Remove _old from texture names

for old_file in *.png; do
    printf "Renaming %s\n" $old_file
    mv $old_file ${old_file%_old.png}.png
done

echo "Finished!"

