#!/bin/bash
echo "RUN"

echo >&2 "testing err"
sleep 10

while read a
do
    echo $a
done

echo "EXIT"
exit 1
