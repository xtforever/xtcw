#!/bin/bash
## set +x


wb_search=${wb_search:-./}

get-superclass()
{
file=$1
super=$(cat $1 | grep "@class" | sed "s/.*(\([^)]*\)).*/\1/g")
echo $super
}


superclass_list=$1
file=$1
src=${1%%.widget}

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
      file=$wb_search/$p.widget
      if ! [ -e $file ]
      then
          break
      fi
  fi

  superclass_list="$file $superclass_list"
done


shift
eval "$@ $superclass_list"
