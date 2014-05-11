#!/usr/bin/env bash
 
source chericc.inc

echo "Reconfiguring libsodium"
pushd "$DIR_SODIUM" > /dev/null
    try_to_run ./autogen.sh
popd > /dev/null

for x in "$DIR_SRC/"*; do
    pushd "$x" > /dev/null
        echo "Reconfiguring $x..."
        try_to_run autoreconf -fvi 
    popd > /dev/null
done

