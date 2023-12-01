#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage <number of clients>"
    exit
fi

# make

mkdir -p analysisFiles/outputs

path="./analysisFiles"
opPath="./analysisFiles/outputs"


cat /dev/null > $path/results.txt


start_time=$(date +%s.%N)

for (( i=1 ; i<=$1 ; i++ )); 
do
    cat /dev/null > $opPath/op$i.txt
    bash $path/check.sh $i > $opPath/op$i.txt &
done

wait

end_time=$(date +%s.%N)

elapsed_time=$(echo "($end_time - $start_time) * 1000000" | bc)

throughput=$(echo "$end_time - $start_time" | bc)
throughput=$(echo "scale=10; $1 / ($end_time - $start_time)" | bc)

# echo $1 $elapsed_time

echo "$elapsed_time $throughput" >> $path/plots/results.txt

rm $opPath/op*.txt

