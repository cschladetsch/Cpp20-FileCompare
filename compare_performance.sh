#!/bin/bash

# Usage: ./compare_performance.sh <file1> <file2>

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <file1> <file2>"
    exit 1
fi

FILE1=$1
FILE2=$2

# Ensure both files exist
if [ ! -f "$FILE1" ] || [ ! -f "$FILE2" ]; then
    echo "Both files must exist."
    exit 1
fi

# Test performance of custom cmp tool
echo "Testing custom cmp tool..."
start_cmp=$(date +%s.%N)
./cmp "$FILE1" "$FILE2" > /dev/null
end_cmp=$(date +%s.%N)
cmp_time=$(echo "$end_cmp - $start_cmp" | bc)
echo "Custom cmp tool time: $cmp_time seconds"

# Test performance of diff command
echo "Testing diff command..."
start_diff=$(date +%s.%N)
diff "$FILE1" "$FILE2" > /dev/null
end_diff=$(date +%s.%N)
diff_time=$(echo "$end_diff - $start_diff" | bc)
echo "diff command time: $diff_time seconds"

