#!/bin/bash

for ((i=1;i<=10;i++))
do
	./skip_plock $i
done

ls
