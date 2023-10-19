#!/bin/bash
echo Analysing and Generating PNG!!

# Different Sizes of Cache
SIZE='
10
40
60
80
100
120
'

if [ $# -ne 3 ]; then
    echo "Usage <loop num> <sleep time> <timeout-secs>"
    exit
fi

cat /dev/null > throughput.txt
cat /dev/null > aat.txt

# kill any ongoing vmstat

# Run the analysis for different sizes of threads
for i in ${SIZE}; do
    echo Testing for $i clients

    vmstat 1 > pref$i.txt &

    bash loadtest.sh ${i} $1 $2 $3  | tee >(awk -v cl=$i '{printf("%f %f\n", cl, $9)}' >> throughput.txt) >(awk -v cl=$i '{printf("%f %f\n", cl, $5)}' >> aat.txt)

    pid=$(ps -eLf | grep vmstat | grep -v grep |  awk '{printf($2)}')
    kill -9 $pid 
done

# Plot the throughput results
cat throughput.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Throughput" -X "Number of Clients" -Y "Throughput" -r 0.25> ./throughput.png


# Plot the average access time results
cat aat.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Average response time" -X "Number of Clients" -Y "Average response time" -r 0.25> ./aat.png

sleep 5

./del.sh

echo Done!!
