#!/bin/bash

for ((i=1;i<=500;i++))
do
	./skip_plock $i
done

ls
