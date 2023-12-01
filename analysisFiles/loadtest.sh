if [ $# -ne 4 ]; then
    echo "Usage <number of clients> <loop num> <sleep time> <timeout-secs>"
    exit
fi

plots_path=./analysisFiles/plots

mkdir -p grader/outputs
rm -f outputs/*

for (( i=1 ; i<=$1 ; i++ )); 
do
    ./client 127.0.0.1 3000 programs/correctp.cpp $2 $3 $i $4 > grader/outputs/op$i.txt &
done

wait


grep "Average" grader/outputs/*.txt | awk  -v nclients="$1" '
    BEGIN{
        sum=0
        total=0
        thru=0
        succ=0
        timeouts=0
        errors=0
        goodput=0
    }
    
    {
        sum=sum+($12*$14)
        total=total+$14
        thru=thru+$27
        goodput=goodput+$40
        succ=succ+$2
        timeouts=timeouts+$32
        errors=errors+$37
    }

    END{
        printf("Average time taken = %f ms. Throughput = %f and Successful = %d of %d | Number of clients = %d Timeout-rate = %d Error-rate = %d Goodput = %f\n", sum/total, thru, succ, total, nclients, timeouts, errors, goodput)
    }
' >> $plots_path/results.txt


