./client new localhost 3000 programs/correctp.cpp $1
# prog id

while true; do
    sleep(5);
    reqID=$(awk '
        {
            print $1;
        }
    ')
    ./client status localhost 3000 $reqID $1

    var=$(grep "processing is done" $1.txt | wc -l)

    if(( var )); then exit 0; fi
done