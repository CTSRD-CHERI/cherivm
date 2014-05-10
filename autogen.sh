#!/usr/bin/env bash
 
source chericc.inc

for x in "$DIR_LIBCHERIJNI" "$DIR_CLASSPATH" "$DIR_JAMVM"; do
    pushd "$x" > /dev/null
        echo "Reconfiguring $x..."
        try_to_run autoreconf -fvi 
    popd > /dev/null
done

pushd "$DIR_SODIUM" > /dev/null
    try_to_run ./autogen.sh
popd > /dev/null
