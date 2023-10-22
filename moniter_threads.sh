#!/bin/bash

cat /dev/null > threads.txt

total=0
n=0
avg=0

while true; do
    threads=$(ps -eLf | grep "./server 3000" | grep -v grep | head -n 1 | awk '{print($6)}')
    
    if [ $threads -ge 0 ]; then
        total=$(($total + $threads))
        n=$(($n + 1))
        avg=$(bc -l <<< "scale=2; $total / $n")

        echo $avg > threads.txt
    fi

    # Sleep for 1 seconds
    sleep 1
done