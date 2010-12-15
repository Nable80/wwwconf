#!/bin/sh

TMP=`mktemp`

for i in *.cpp *.h; do
    awk -f "$1" "$i" > "$TMP" && mv "$TMP" "$i";
done;
