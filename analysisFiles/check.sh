#!/bin/bash

mkdir -p analysisFiles/clientFiles

path="./analysisFiles/clientFiles"
filename="$1.txt"

cat /dev/null > $path/$filename

./client new localhost 3000 programs/comperr.cpp "$1"

while true; do
    sleep 1;

    if [ ! -s "$path/$filename" ]; then
        # echo "The file is empty."
        exit 0
    fi

    reqID=$(awk '{print $1}' "$path/$filename")

    ./client status localhost 3000 "$reqID" "$1"

    var=$(grep "processing is done" "$path/status$filename" | wc -l)

    if [ $var -gt 0 ]; then
        break
    fi
done

echo done
