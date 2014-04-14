#!/usr/bin/env bash
 
source chericc.inc

for x in "$DIR_SRC/jamvm"; do
    pushd "$x" > /dev/null
        echo "Reconfiguring $x..."
        try_to_run autoreconf -fvi 
    popd > /dev/null
done

