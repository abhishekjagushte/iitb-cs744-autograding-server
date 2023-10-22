if [ $# -ne 4 ]; then
    echo "Usage <number of clients> <loop num> <sleep time> <timeout-secs>"
    exit
fi

gcc -o client gradingclient.c
mkdir -p outputs
rm -f outputs/*

for (( i=1 ; i<=$1 ; i++ )); 
do
    ./client 127.0.0.1 3000 correctp.cpp $2 $3 $i $4 > outputs/op$i.txt &
done

wait


grep "Average" outputs/*.txt | awk  -v nclients="$1" '
    BEGIN{
        sum=0
        total=0
        thru=0
        succ=0
        timeouts=0
    }
    
    {
        sum=sum+($12*$14)
        total=total+$14
        thru=thru+$27
        succ=succ+$2
        timeouts=timeouts+$31
    }

    END{
        printf("Average time taken = %f ms. Throughput = %f and Successful = %d of %d | Number of clients = %d Timeouts = %d\n", sum/total, thru, succ, total, nclients, timeouts)
    }
' >> results.txt


