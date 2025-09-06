#!/bin/bash
avg=0
for num in "$@"
do
    avg=$((avg+num))
done
avg=$((avg/$#))
echo $avg