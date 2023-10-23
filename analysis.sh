#!/bin/bash
echo Analysing and Generating PNG!!
echo

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
cat /dev/null > error_rate.txt
cat /dev/null > timeout_rate.txt
cat /dev/null > succ_rate.txt
cat /dev/null > req_rate.txt
cat /dev/null > loadUti.txt


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

    cat results.txt | awk '{printf("%d %d\n", $21, $24)}' >> timeout_rate.txt
    cat results.txt | awk '{printf("%d %d\n", $21, $27)}' >> error_rate.txt
    cat results.txt | awk '{printf("%d %d\n", $21, $9)}' >> succ_rate.txt
    cat results.txt | awk '{printf("%d %d\n", $21, $24 + $27 + $9)}' >> req_rate.txt

    pkill -f './moniter_threads.sh'
    pkill -f 'vmstat 1'
    wait $! 2>/dev/null

    # calculate the averages and save to a file
    avg_threads=$(cat threads.txt)
    echo $i $avg_threads >> threadsplot.txt

    sed -n /^[0-9]/p pref$i.txt | awk '{print $15}' > uti$i.txt
    
    echo -n "${i} " >> loadUti.txt
    bash avg_uti.sh $i | grep -o "[0-9]*\.[0-9]*" >> loadUti.txt
    
    cat /dev/null > results.txt

    echo
done


# Plot the throughput results
cat throughput.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Throughput" -X "Number of Clients" -Y "Throughput" -r 0.25> ./throughput.png

# Plot the average request time results
cat aat.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Average response time" -X "Number of Clients" -Y "Average response time" -r 0.25> ./aat.png

# Plot the average active threads results
cat threadsplot.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Average active threads" -X "Number of Clients" -Y "Average active threads" -r 0.25> ./threads.png

# Plot the error rate results
cat error_rate.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Error Rate" -X "Number of Clients" -Y "Error Rate" -r 0.25> ./error_rate.png

# Plot the timeout rate results
cat timeout_rate.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Timeout Rate" -X "Number of Clients" -Y "Timeout Rate" -r 0.25> ./timeout_rate.png

# Plot the goodput results
cat succ_rate.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Success Rate" -X "Number of Clients" -Y "Goodput" -r 0.25> ./succ_rate.png

# Plot the Request rate results
cat req_rate.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Request Rate" -X "Number of Clients" -Y "Request Rate" -r 0.25> ./req_rate.png

# Plot the CPU Utilisation results
cat loadUti.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs CPU Utilisation" -X "Number of Clients" -Y "CPU Utilisation" -r 0.25> ./cpu_uti.png

sleep 5

./del.sh

echo Done!!
