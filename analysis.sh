#!/bin/bash
echo Analysing and Generating PNG!!
echo

chmod +x analysisFiles/moniter_threads.sh

plots_path=./analysisFiles/plots

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
pkill -f './analysisFiles/moniter_threads.sh'
wait $! 2>/dev/null

make client --silent

# Run the analysis for different sizes of threads
for i in ${SIZE}; do
    echo Testing for $i clients

    vmstat 1 > $plots_path/pref$i.txt &
    ./analysisFiles/moniter_threads.sh &

    bash analysisFiles/loadtest.sh ${i} $1 $2 $3
    cat $plots_path/results.txt | tee >(awk -v cl=$i '{printf("%f %f\n", cl, $9)}' >> $plots_path/throughput.txt) >(awk -v cl=$i '{printf("%f %f\n", cl, $5)}' >> $plots_path/aat.txt)

    cat $plots_path/results.txt | awk '{printf("%d %d\n", $21, $24)}' >> $plots_path/timeout_rate.txt
    cat $plots_path/results.txt | awk '{printf("%d %d\n", $21, $27)}' >> $plots_path/error_rate.txt
    cat $plots_path/results.txt | awk '{printf("%d %d\n", $21, $9)}' >> $plots_path/succ_rate.txt
    cat $plots_path/results.txt | awk '{printf("%d %d\n", $21, $24 + $27 + $9)}' >> $plots_path/req_rate.txt

    pkill -f './analysisFiles/moniter_threads.sh'
    pkill -f 'vmstat 1'
    wait $! 2>/dev/null

    # calculate the averages and save to a file
    avg_threads=$(cat $plots_path/threads.txt)
    echo $i $avg_threads >> $plots_path/threadsplot.txt

    avg_uti=$(bash analysisFiles/avg_uti.sh $i | grep -o "[0-9]*\.[0-9]*")
    echo $i $avg_uti >> $plots_path/loadUti.txt
    
    cat /dev/null > $plots_path/results.txt

    rm -f /grader/*

    echo ""

done


FILENAME='
throughput
aat
threadsplot
error_rate
timeout_rate
succ_rate
req_rate
loadUti
'

for i in ${FILENAME}; do
    cat $plots_path/$i.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs $i" -X "Number of Clients" -Y "$i" -r 0.25 > $plots_path/lab9/$i.png
done


for i in ${FILENAME}
do
    file="$plots_path/lab8/$i.txt"
    gnuplot << EOF
    set terminal png
    set output "$plots_path/$i.png"
    set title "Number of Clients vs $i -- Combined"
    set xlabel "Number of Clients"
    set ylabel "$i"
    set grid

    plot "$plots_path/$i.txt" using 1:2 with linespoints title "Response Time ver3" linecolor rgb "blue", \
    "$file" using 1:2 with linespoints title "Response Time ver1" linecolor rgb "red"
EOF
done

sleep 10

./analysisFiles/del.sh

echo Done!!
