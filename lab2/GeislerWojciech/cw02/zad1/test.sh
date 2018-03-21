#!/bin/bash -e

COMMAND=$1
OUT=${2:-times.txt}
BLOCKS=1000
BLOCKS2=3300 # insertions sort is O(n^2), increase by log2(10)

cat /dev/null > $OUT

for rs in {4,512,4096,8192}; do
    for bs in {$BLOCKS,$BLOCKS2}; do
        echo $bs $rs
        $COMMAND generate generated.bin $bs $rs sys
        $COMMAND copy generated.bin copy.bin $bs $rs sys >> $OUT
        $COMMAND copy generated.bin copy2.bin $bs $rs lib >> $OUT
        diff -q copy.bin copy2.bin
        echo "sort $bs blocks of $rs bytes using lib"
        $COMMAND sort copy.bin $bs $rs lib >> $OUT
        echo "sort $bs blocks of $rs bytes using sys"
        $COMMAND sort copy2.bin $bs $rs sys >> $OUT
    done
done
