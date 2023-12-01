#!/bin/bash
echo Analysing and Generating PNG!!
echo

chmod +x moniter_threads.sh

plots_path=./plots

# Different Sizes of Cache
SIZE='
10
40
60
80
100
120
150
180
'

if [ $# -ne 3 ]; then
    echo "Usage <loop num> <sleep time> <timeout-secs>"
    exit
fi


cat /dev/null > $plots_path/throughput.txt
cat /dev/null > $plots_path/aat.txt
cat /dev/null > $plots_path/threadsplot.txt
cat /dev/null > $plots_path/results.txt
cat /dev/null > $plots_path/error_rate.txt
cat /dev/null > $plots_path/timeout_rate.txt
cat /dev/null > $plots_path/succ_rate.txt
cat /dev/null > $plots_path/req_rate.txt
cat /dev/null > $plots_path/loadUti.txt


# kill any ongoing vmstat
pkill -f 'vmstat 1'
pkill -f './moniter_threads.sh'
wait $! 2>/dev/null

# Run the analysis for different sizes of threads
for i in ${SIZE}; do
    echo Testing for $i clients

    vmstat 1 > $plots_path/pref$i.txt &
    ./moniter_threads.sh &

    bash loadtest.sh ${i} $1 $2 $3
    cat $plots_path/results.txt | tee >(awk -v cl=$i '{printf("%f %f\n", cl, $9)}' >> $plots_path/throughput.txt) >(awk -v cl=$i '{printf("%f %f\n", cl, $5)}' >> $plots_path/aat.txt)

    cat $plots_path/results.txt | awk '{printf("%d %d\n", $21, $24)}' >> $plots_path/timeout_rate.txt
    cat $plots_path/results.txt | awk '{printf("%d %d\n", $21, $27)}' >> $plots_path/error_rate.txt
    cat $plots_path/results.txt | awk '{printf("%d %d\n", $21, $9)}' >> $plots_path/succ_rate.txt
    cat $plots_path/results.txt | awk '{printf("%d %d\n", $21, $24 + $27 + $9)}' >> $plots_path/req_rate.txt

    pkill -f './moniter_threads.sh'
    pkill -f 'vmstat 1'
    wait $! 2>/dev/null

    # calculate the averages and save to a file
    avg_threads=$(cat $plots_path/threads.txt)
    echo $i $avg_threads >> $plots_path/threadsplot.txt

    avg_uti=$(bash avg_uti.sh $i | grep -o "[0-9]*\.[0-9]*")
    echo $i $avg_uti >> $plots_path/loadUti.txt
    
    cat /dev/null > $plots_path/results.txt

    echo
done


# Plot the throughput results
cat $plots_path/throughput.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Throughput" -X "Number of Clients" -Y "Throughput" -r 0.25> $plots_path/throughput.png

# Plot the average request time results
cat $plots_path/aat.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Average response time" -X "Number of Clients" -Y "Average response time" -r 0.25> $plots_path/aat.png

# Plot the average active threads results
cat $plots_path/threadsplot.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Average active threads" -X "Number of Clients" -Y "Average active threads" -r 0.25> $plots_path/threads.png

# Plot the error rate results
cat $plots_path/error_rate.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Error Rate" -X "Number of Clients" -Y "Error Rate" -r 0.25> $plots_path/error_rate.png

# Plot the timeout rate results
cat $plots_path/timeout_rate.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Timeout Rate" -X "Number of Clients" -Y "Timeout Rate" -r 0.25> $plots_path/timeout_rate.png

# Plot the goodput results
cat $plots_path/succ_rate.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Success Rate" -X "Number of Clients" -Y "Goodput" -r 0.25> $plots_path/succ_rate.png

# Plot the Request rate results
cat $plots_path/req_rate.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Request Rate" -X "Number of Clients" -Y "Request Rate" -r 0.25> $plots_path/req_rate.png

# Plot the CPU Utilisation results
cat $plots_path/loadUti.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs CPU Utilisation" -X "Number of Clients" -Y "CPU Utilisation" -r 0.25> $plots_path/cpu_uti.png

sleep 5

./del.sh

echo Done!!
