#!/bin/sh

for dist in slackware; do
	for vermaj in `seq 7 7`; do
		for vermin in `seq 1 1`; do
			for arch in i386; do
				./dist-id $dist $arch $vermaj $vermin
			done
		done
	done
done
