cat /dev/null > clientFiles/$1.txt

start_time=$(date +%s.%N)

./client new localhost 3000 programs/comperr.cpp "$1"

while true; do
    # sleep 5;
    reqID=$(awk '{print $1}' "clientFiles/$1.txt")

    ./client status localhost 3000 "$reqID" "$1"

    var=$(grep "processing is done" "clientFiles/status$1.txt" | wc -l)

    if [ $var -gt 0 ]; then
        break
    fi
done

end_time=$(date +%s.%N)
elapsed_time=$(echo "($end_time - $start_time) * 1000000" | bc)

echo "Average Resp time: $elapsed_time us"