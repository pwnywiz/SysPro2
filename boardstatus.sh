#!/bin/bash

i=0
j=0
flag=0

for elem in $(find $1 -mindepth 1 -maxdepth 1 -type d)
do
  for file in $(find $elem -mindepth 1 -maxdepth 1 -name '_pid' -type f)
  do
    serverpid=$(cat "$file")
    for temp in $(ps -a | grep "board" | awk '{print $1}')
    do
      if (("$serverpid" == "$temp"))
      then
        running[$i]=$elem
        i=$(($i+1))
        flag=1
      fi
    done
    if (("$flag" == "0"))
    then
      deadb[$j]=$elem
      j=$(($j+1))
    fi
    if (("$flag" == "1"))
    then
      flag=0
    fi
  done
done

echo $i Active Boards
echo $running
echo
echo $j Dead Boards
echo $dead
