#!/bin/bash

if [ $# -ne 3 ]; then
    echo "check.sh: Usage <prog_id> <host> <port>"
    exit
fi

mkdir -p analysisFiles/clientFiles

path="./analysisFiles/clientFiles"
filename="$1.txt"

cat /dev/null > $path/$filename

./client new $2 $3 programs/comperr.cpp "$1"

while true; do
    sleep 1;

    if [ ! -s "$path/$filename" ]; then
        # echo "The file is empty."
        exit 0
    fi

    reqID=$(awk '{print $1}' "$path/$filename")

    ./client status $2 $3 "$reqID" "$1"

    var=$(grep "processing is done" "$path/status$filename" | wc -l)

    if [ $var -gt 0 ]; then
        break
    fi
done

echo done
