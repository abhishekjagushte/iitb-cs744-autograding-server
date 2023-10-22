#!/bin/bash
echo Analysing and Generating PNG!!

chmod +x moniter_threads.sh

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
cat /dev/null > threadsplot.txt
cat /dev/null > results.txt


# kill any ongoing vmstat
pkill -f 'vmstat 1'
pkill -f './moniter_threads.sh'
wait $! 2>/dev/null

# Run the analysis for different sizes of threads
for i in ${SIZE}; do
    echo Testing for $i clients

    vmstat 1 > pref$i.txt &
    ./moniter_threads.sh &

    bash loadtest.sh ${i} $1 $2 $3
    cat results.txt | tee >(awk -v cl=$i '{printf("%f %f\n", cl, $9)}' >> throughput.txt) >(awk -v cl=$i '{printf("%f %f\n", cl, $5)}' >> aat.txt)

    cat results.txt | awk '{printf("%d %d\n", $21, 100 - ($24/$15)*100)}' >> error_rate.txt


    pkill -f './moniter_threads.sh'
    pkill -f 'vmstat 1'
    wait $! 2>/dev/null

    # calculate the averages and save to a file
    avg_threads=$(cat threads.txt)
    echo $i $avg_threads >> threadsplot.txt

    sed -n /^[0-9]/p pref$i.txt | awk '{print $15}' > uti$i.txt
    bash avg_uti.sh $i
    cat /dev/null > results.txt
done


# Plot the throughput results
cat throughput.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Throughput" -X "Number of Clients" -Y "Throughput" -r 0.25> ./throughput.png


# Plot the average access time results
cat aat.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Average response time" -X "Number of Clients" -Y "Average response time" -r 0.25> ./aat.png

# Plot the average active threads results
cat threadsplot.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Average active threads" -X "Number of Clients" -Y "Average active threads" -r 0.25> ./threads.png

# Plot the error rate results
cat error_rate.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Error Rate" -X "Number of Clients" -Y "Error Rate" -r 0.25> ./error_rate.png


sleep 5

./del.sh

echo Done!!
