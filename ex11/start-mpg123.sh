#!/bin/bash
if [ "$1" == "mpg123" ] 
then
  killall mpg123
  mpg123 test -R 2>&1
  exit 0
fi
while true
do
	socat tcp-l:12300,reuseaddr EXEC:"$0 mpg123"
done
