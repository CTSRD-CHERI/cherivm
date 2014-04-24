#!/usr/bin/env bash
 
source chericc.inc

if [[ x"$1" == x ]]; then
    TARGET="install"
else
    TARGET="$1"
fi

for x in "$DIR_LIBCHERIJNI" "$DIR_CLASSPATH" "$DIR_JAMVM"; do
    pushd "$x" > /dev/null
        gmake "$TARGET" || exit
    popd > /dev/null
done

