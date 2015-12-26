#!/bin/sh

PROGRAM="/home/tim/work/my/karaokeplayer/build/debug/src/karaokeplayer"

ulimit -c unlimited
mkdir cores

while [ true ]; do

    $PROGRAM -fs
    
    if [ $? == 0 ]; then
        exit;
    fi

    # Find core, if any
    core=`find . -maxdepth 1 -type f -name core\*`
    date=`date`
    
    if [ -n "$core" ]; then
        gdb -batch -ex "thread apply all bt full" $PROGRAM $core 2>&1
        mv $core cores/"$date.core"
    fi
    
done
