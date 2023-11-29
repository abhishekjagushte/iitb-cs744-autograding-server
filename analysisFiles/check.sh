./client new localhost 3000 programs/comperr.cpp "$1"
# prog id
count=0
while true; do
    # sleep 5;
    reqID=$(awk '{print $1}' "clientFiles/$1.txt")

    ./client status localhost 3000 "$reqID" "$1"

    var=$(grep "processing is done" "clientFiles/status$1.txt" | wc -l)

    if [ $var -gt 0 ]; then
        exit 0
    fi
done