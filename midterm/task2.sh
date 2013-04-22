#!/bin/bash -e

FILE="$1"

prev=""

while read line
do
    if [[ $prev =~ ^[0-9]+$ && $line =~ ^[0-9]+$ ]]
    then
        sum=$((prev + line))
        if [ $sum -gt 10 ]; then
            echo $prev
            echo $line
            echo "----"
        fi
    fi
    prev="$line"
done < "$FILE"

