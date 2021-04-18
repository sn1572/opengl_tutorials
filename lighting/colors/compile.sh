#!/bin/bash

module load glad/1.0
module load learnOpengl/1.0

NAME="$( echo "$1" | cut -d'.' -f1 )"

LOGS=(log_compile.txt log_link.txt)

for file in ${LOGS[@]}
do
	if test -f $file
	then
		rm $file
		touch $file
	else
		touch $file
	fi
done
 
gcc -g -c -o $NAME.o $1 > log_compile.txt 2>&1
if test -f $NAME.o
then
	gcc -o $NAME.exe $NAME.o -Wl,-rpath,$LD_LIBRARY_PATH -lshader -lcamera -lglfw -lGL -lglad -ldl -lm > log_link.txt 2>&1
fi

if test -f ${NAME}.o
then
	rm $NAME.o
fi

for file in ${LOGS[@]}
do
	if test -f $file
	then
		#RED='\033[0;31m'
		#NC='\033[0m'
		#awk '/error/ {print}' log_compile.txt | sed -i -e 's/error/${RED}error${NC}/g'
		awk '/error/ {print}' $file
	fi
done
