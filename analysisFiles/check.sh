path="./clientFiles"
filename="$1.txt"

cat /dev/null > $path/$filename

start_time=$(date +%s.%N)

./client new localhost 3000 programs/comperr.cpp "$1"

while true; do
    sleep 5;

    if [ ! -s "$path/$filename" ]; then
        # echo "The file is empty."
        exit 0
    fi

    reqID=$(awk '{print $1}' "$path/$filename")

    ./client status localhost 3000 "$reqID" "$1"

    var=$(grep "processing is done" "$path/status$filename" | wc -l)

    if [ $var -gt 0 ]; then
        break
    fi
done

end_time=$(date +%s.%N)
elapsed_time=$(echo "($end_time - $start_time) * 1000000" | bc)

echo "Average Resp time: $elapsed_time us"