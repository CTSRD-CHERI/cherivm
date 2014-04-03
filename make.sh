#!/usr/bin/env bash
 
source chericc.inc
  
for x in "$DIR_SRC/"*; do
    pushd "$x" > /dev/null
        gmake $1 || exit
        gmake install || exit
    popd > /dev/null
done

