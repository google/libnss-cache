#!/bin/bash

in=$1
column=$2
out=$3

map=$(mktemp /tmp/libnss.XXXXXX)
strings -at d "$in" > "$map"
: > "$out"

exec 2>/dev/null
for value in $(tr -s [:blank:] ! < "$map" | cut -f3 -d! | cut -f"$column" -d: | LC_COLLATE=C sort) ; do
    offset=$(grep "$value" "$map" | tr -s [:blank:] ! | cut -f2 -d!)
    printf "%s\0%d\0" "$value" "$offset" >> "$out"
    len=$(printf "%s\0%d\0" "$value" "$offset" | wc -c)
    for _i in $(seq 1 $((16-len))) ; do 
        printf "\0" >> "$out"
    done
    printf "\n" >> "$out"
done
rm "$map"
