#!/bin/bash

for ((i=1;i<=64;i*=2))
do
	echo "Do random, number of threads : $i"
	./skip_plock rnd100000 $i > nolock_rnd_noft_$i
	echo "Done"
done

for ((i=1;i<=64;i*=2))
do
	echo "Do random, number of threads : $i"
	./skip_wlock rnd100000 $i > lock_rnd_noft_$i
	echo "Done"
done

echo "random done"

for ((i=1;i<=64;i*=2))
do
	echo "Do sequential, number of threads : $i"
	./skip_plock seq10000000 $i > nolock_seq_noft_$i
	echo "Done"
done

for ((i=1;i<=64;i*=2))
do
	echo "Do sequential, number of threads : $i"
	./skip_wlock seq10000000 $i > lock_seq_noft_$i
	echo "Done"
done

echo "sequential done"
