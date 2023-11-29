plots_path=./analysisFiles/plots

# mkdir -p grader/outputs
# rm -f outputs/*

# result=$(time ./analysisFiles/check.sh 1 2>&1)

{ time ./test_client.sh 0.0.0.0 8080 studentCode.cpp 20 ; } 2>&1 | grep "real" | awk 'BEGIN {FS="\t"} {print $2}'

# echo $result


# >> $plots_path/results.txt


