#!/bin/bash
#set -x

get-superclass()
{
file=$1
super=$(cat $1 | grep "@class" | sed "s/.*(\([^)]*\)).*/\1/g")
echo $super
}


superclass_list=$1
file=$1
src=${1%%.widget}
rm src/$src.c
while true
do
  p=$(get-superclass $file)
  if [ -z $p ]
  then
      break;
  fi

  file=$p.widget
  if ! [ -e $file ]
  then
    break;
  fi

  superclass_list="$file $superclass_list"
done


shift
eval "$@ $superclass_list"
