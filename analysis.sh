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
    cat results.txt | tee >(awk -v cl=$i '{printf("%f %f\n", cl, $9)}' >> throughput.txt) >(awk -v cl=$i '{printf("%d %f\n", cl, $5)}' >> aat.txt)

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

    avg_uti=$(bash avg_uti.sh $i | grep -o "[0-9]*\.[0-9]*")
    echo $i $avg_uti >> loadUti.txt
    
    cat /dev/null > results.txt

    echo
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
    cat $i.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs $i" -X "Number of Clients" -Y "$i" -r 0.25> ./$i.png

done


for i in ${FILENAME}; do
#    cat $i.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs $i" -X "Number of Clients" -Y "$i" -r 0.25> ./$i.png
    
    file="$(i)_8.txt"
    gnuplot << EOF
    set terminal png
    set output "$i.png"
    set title "Number of Clients vs $i -- Combined"
    set xlabel "$i"
    set ylabel "Number of Clients"
    set grid

    plot "$i.txt" using 1:2 with linespoints title "Response Time ver3" linecolor rgb "blue", \
         "$file" using 1:2 with linespoints title "Response Time ver1" linecolor rgb "red" 
    EOF
done

sleep 5

./del.sh

echo Done!!
