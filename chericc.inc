#!/usr/bin/env bash

source "config.inc"

DIR_CHERISDK_BIN="$DIR_CHERISDK/bin"

export CC="$DIR_CHERISDK_BIN/cheri-unknown-freebsd-clang"
export CXX="$DIR_CHERISDK_BIN/cheri-unknown-freebsd-clang++"
export CFLAGS="-g -integrated-as --sysroot=$DIR_CHERISYS -B$DIR_CHERISDK"
export CCASFLAGS="$CFLAGS -no-integrated-as" # don't use -integrated-as with .S files
export LDFLAGS="-Wl,-melf64btsmip_fbsd -Wl,-g"

